#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define CHANNEL ADC1_CHANNEL_3

#define LED_GROEN 18

#define PWM_TIMER LEDC_TIMER_0
#define PWM_MODE LEDC_LOW_SPEED_MODE
#define PWM_CHANNEL LEDC_CHANNEL_0
#define PWM_DUTY_RES LEDC_TIMER_13_BIT
#define PWM_MAX_DUTY ((1 << 13) - 1)
#define PWM_FREQ_HZ 200

static int clamp_int(int value, int min, int max)
{
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static void pwm_init_led(void)
{
    ledc_timer_config_t timer_cfg = {
        .speed_mode = PWM_MODE,
        .duty_resolution = PWM_DUTY_RES,
        .timer_num = PWM_TIMER,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_cfg);

    ledc_channel_config_t channel_cfg = {
        .gpio_num = LED_GROEN,
        .speed_mode = PWM_MODE,
        .channel = PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = PWM_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&channel_cfg);
}

void app_main(void)
{
    pwm_init_led();
    
    int potWaarde = 0;
    int rawWaarde = 0;
    static esp_adc_cal_characteristics_t *adc_chars;
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(CHANNEL, ADC_ATTEN_DB_12);
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, adc_chars);

    while(1)
    {
        //Potmeter inlezen
        rawWaarde = adc1_get_raw((adc1_channel_t)CHANNEL);
        potWaarde = esp_adc_cal_raw_to_voltage(rawWaarde, adc_chars);
        potWaarde = clamp_int(potWaarde, 0, 3300);

        int duty = (potWaarde * PWM_MAX_DUTY) / 3300;
        int duty_procent = (duty * 100 + (PWM_MAX_DUTY / 2)) / PWM_MAX_DUTY;
        ledc_set_duty(PWM_MODE, PWM_CHANNEL, duty);
        ledc_update_duty(PWM_MODE, PWM_CHANNEL);

        printf("ADC raw: %4d | Licht: %4d mV | LED duty: %4d/%d (%3d%%)\n", rawWaarde, potWaarde, duty, PWM_MAX_DUTY, duty_procent);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

}