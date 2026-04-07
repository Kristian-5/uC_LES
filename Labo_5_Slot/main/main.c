#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define LED_LOCK 18
#define BTN_LOCK 4

#define LED_CORRECT 38

#define CHANNEL ADC1_CHANNEL_4

void app_main(void)
{
    gpio_reset_pin(LED_LOCK);
    gpio_set_direction(LED_LOCK, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_CORRECT);
    gpio_set_direction(LED_CORRECT, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BTN_LOCK);
    gpio_set_direction(BTN_LOCK, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BTN_LOCK, GPIO_PULLDOWN_ONLY);

    static esp_adc_cal_characteristics_t *adc_chars;
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(CHANNEL, ADC_ATTEN_DB_11);

    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(
        ADC_UNIT_1,
        ADC_ATTEN_DB_11,
        ADC_WIDTH_BIT_12,
        1100,
        adc_chars);

    int code[4] = {0, 0, 0, 0};
    int guess[4] = {0, 0, 0, 0};

    int codeIndex = 0;
    int guessIndex = 0;

    bool codeIsSet = false;
    bool slotOpen = false;

    while (1)
    {
        int potmV = esp_adc_cal_raw_to_voltage(adc1_get_raw(CHANNEL), adc_chars);
        int getal = potmV / 330; // 0–9
        if (getal > 9)
            getal = 9;

        printf("ADC getal: %d\n", getal);

        if (slotOpen)
            gpio_set_level(LED_LOCK, 0); 
        else
            gpio_set_level(LED_LOCK, 1);

        if (!codeIsSet)
        {
            if (gpio_get_level(BTN_LOCK) == 1)
            {
                code[codeIndex] = getal;
                codeIndex++;

                printf("Code digit %d = %d\n", codeIndex, getal);
                vTaskDelay(300 / portTICK_PERIOD_MS);

                if (codeIndex == 4)
                {
                    codeIsSet = true;
                    printf("CODE COMPLEET!\n");
                }
            }
        }
        else
        {
            if (gpio_get_level(BTN_LOCK) == 1)
            {
                guess[guessIndex] = getal;
                guessIndex++;

                printf("Guess digit %d = %d\n", guessIndex, getal);
                vTaskDelay(300 / portTICK_PERIOD_MS);

                if (guessIndex == 4)
                {
                    if (code[0] == guess[0] &&
                        code[1] == guess[1] &&
                        code[2] == guess[2] &&
                        code[3] == guess[3])
                    {
                        printf("CORRECTE CODE!\n");
                        slotOpen = true; // LED blijft uit
                        gpio_set_level(LED_CORRECT, 1);
                    }
                    else
                    {
                        printf("FOUTE CODE!\n");
                        gpio_set_level(LED_CORRECT, 0);

                        // LED knipperen (slot blijft dicht)
                        for (int i = 0; i < 5; i++)
                        {
                            gpio_set_level(LED_LOCK, 0);
                            vTaskDelay(200 / portTICK_PERIOD_MS);
                            gpio_set_level(LED_LOCK, 1);
                            vTaskDelay(200 / portTICK_PERIOD_MS);
                        }
                    }
                    guessIndex = 0;
                }
            }
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}