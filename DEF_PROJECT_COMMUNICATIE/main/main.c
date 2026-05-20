#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "myGAS.h"
#include "driver/gpio.h"

#define LED_BLUE 18

void app_main(void)
{
    struct myGAS_config config;

    config.port = MYGAS_I2C_PORT;
    config.sda_pin = MYGAS_SDA_PIN;
    config.scl_pin = MYGAS_SCL_PIN;

    struct myGAS_data gasData;

    if (myGAS_init(&config) != ESP_OK)
    {
        printf("Sensor werkt niet\n");
        return;
    }

    printf("Sensor gestart!\n\n");


    gpio_reset_pin(LED_BLUE);
    gpio_set_direction(LED_BLUE, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_BLUE, 0);

    while (1)
    {
        if (myGAS_read(&gasData) == ESP_OK)
        {
            printf("------ GAS SENSOR ------\n");

            printf("Temperatuur: %.1f C\n",
                   gasData.temperature_c);

            printf("Vochtigheid: %.1f %%\n",
                   gasData.humidity_percent);

            printf("TVOC: %d ppb\n",
                   gasData.tvoc_ppb);

            printf("eCO2: %d ppm\n\n",
                   gasData.eco2_ppm);
                vTaskDelay(pdMS_TO_TICKS(100));

                gpio_set_level(LED_BLUE, 1);
                vTaskDelay(pdMS_TO_TICKS(100));
                gpio_set_level(LED_BLUE, 0);
                

        }
        else
        {
            printf("Fout bij uitlezen sensor\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}