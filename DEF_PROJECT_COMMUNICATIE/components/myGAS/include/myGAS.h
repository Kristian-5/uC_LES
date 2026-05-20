#include <stdint.h>
#include "esp_err.h"

// standaard pins voor i2c
#define MYGAS_I2C_PORT 0
#define MYGAS_SDA_PIN 8
#define MYGAS_SCL_PIN 9

struct myGAS_config
{
    int port;
    int sda_pin;
    int scl_pin;
};

// hier komen de waarden die we uitlezen
struct myGAS_data
{
    float temperature_c;
    float humidity_percent;
    uint16_t tvoc_ppb;
    uint16_t eco2_ppm;
};

// functies die ik ga maken in de .c file
esp_err_t myGAS_init(struct myGAS_config *config);
esp_err_t myGAS_read(struct myGAS_data *data);