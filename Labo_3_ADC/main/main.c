#include <stdio.h>
#include "myADC.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_GROEN 38
#define LED_ROOD  37
#define LED_BLAUW 36

#define CHANNEL ADC_CHANNEL_4

static void leds_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GROEN) | (1ULL << LED_ROOD) | (1ULL << LED_BLAUW),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);

    gpio_set_level(LED_GROEN, 0);
    gpio_set_level(LED_ROOD, 0);
    gpio_set_level(LED_BLAUW, 0);
}

static void zet_kleur(float temp)
{
    if (temp > 19.0f) {
        gpio_set_level(LED_ROOD, 1);
        gpio_set_level(LED_GROEN, 0);
        gpio_set_level(LED_BLAUW, 0);
    }
    else if (temp >= 18.0f) {
        gpio_set_level(LED_ROOD, 0);
        gpio_set_level(LED_GROEN, 1);
        gpio_set_level(LED_BLAUW, 0);
    }
    else {
        gpio_set_level(LED_ROOD, 0);
        gpio_set_level(LED_GROEN, 0);
        gpio_set_level(LED_BLAUW, 1);
    }
}

void app_main(void)
{
    leds_init();
    myADC_setup(CHANNEL);

    while (1)
    {
        int mv = myADC_getMiliVolt(CHANNEL);
        float temp = mv / 10.0f;

        zet_kleur(temp);

        printf("Spanning: %d mV | Temp: %.1f °C\n", mv, temp);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
 