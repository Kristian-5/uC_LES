#include "myGAS.h"

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ENS160_ADDR 0x53
#define AHT21_ADDR  0x38
#define I2C_SPEED   100000

static i2c_master_bus_handle_t bus;
static i2c_master_dev_handle_t ens160;
static i2c_master_dev_handle_t aht21;

static esp_err_t add_i2c_device(uint8_t address, i2c_master_dev_handle_t *device)
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = I2C_SPEED,
    };

    return i2c_master_bus_add_device(bus, &dev_cfg, device);
}

esp_err_t myGAS_init(struct myGAS_config *config)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = config->port,
        .sda_io_num = config->sda_pin,
        .scl_io_num = config->scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .flags.enable_internal_pullup = true,
    };

    if (i2c_new_master_bus(&bus_cfg, &bus) != ESP_OK) return ESP_FAIL;
    if (add_i2c_device(ENS160_ADDR, &ens160) != ESP_OK) return ESP_FAIL;
    if (add_i2c_device(AHT21_ADDR, &aht21) != ESP_OK) return ESP_FAIL;

    // ENS160 in standaard mode zetten
    uint8_t ens_mode[] = {0x10, 0x02};
    if (i2c_master_transmit(ens160, ens_mode, sizeof(ens_mode), -1) != ESP_OK) return ESP_FAIL;

    vTaskDelay(pdMS_TO_TICKS(100));
    return ESP_OK;
}

esp_err_t myGAS_read(struct myGAS_data *data)
{
    uint8_t aht_cmd[] = {0xAC, 0x33, 0x00};
    uint8_t aht[6];

    if (i2c_master_transmit(aht21, aht_cmd, sizeof(aht_cmd), -1) != ESP_OK) return ESP_FAIL;
    vTaskDelay(pdMS_TO_TICKS(80));
    if (i2c_master_receive(aht21, aht, sizeof(aht), -1) != ESP_OK) return ESP_FAIL;

    uint32_t raw_hum = ((uint32_t)aht[1] << 12) | ((uint32_t)aht[2] << 4) | (aht[3] >> 4);
    uint32_t raw_tmp = (((uint32_t)aht[3] & 0x0F) << 16) | ((uint32_t)aht[4] << 8) | aht[5];

    data->humidity_percent = raw_hum * 100.0f / 1048576.0f;
    data->temperature_c = raw_tmp * 200.0f / 1048576.0f - 50.0f;

    uint8_t reg = 0x21;
    uint8_t air[5];

    if (i2c_master_transmit_receive(ens160, &reg, 1, air, sizeof(air), -1) != ESP_OK) return ESP_FAIL;

    data->tvoc_ppb = air[1] | (air[2] << 8);
    data->eco2_ppm = air[3] | (air[4] << 8);

    return ESP_OK;
}
