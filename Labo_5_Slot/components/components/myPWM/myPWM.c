#include <stdio.h>
#include "myPWM.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

static ledc_channel_config_t ledc_channel = 
{
        .channel = LEDC_CHANNEL_0,  
        .duty = 0,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_0,
};

static ledc_timer_config_t ledc_timer = 
    {
        .duty_resolution = LEDC_TIMER_10_BIT, // resolution of PWM duty
        .freq_hz = 1000,                      // frequency of PWM signal
        .speed_mode = LEDC_HIGH_SPEED_MODE,    // timer mode
        .timer_num = LEDC_TIMER_0,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock.
    };
void myPWM_Initialize(int gpioPin)
{
    ledc_channel.gpio_num = gpioPin;
    

    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&ledc_channel);
}

void myPWM_SetDuty(int value)
{
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, value);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
}

void myPWM_Frequency(int freq)
{
    ledc_timer.freq_hz = freq;
    ledc_timer_config(&ledc_timer);
}