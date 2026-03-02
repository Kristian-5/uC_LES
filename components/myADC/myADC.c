#include <stdio.h>
#include <stdbool.h>
#include "myADC.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define ADC_CALIBRATION 1
#define AANTAL_STALEN 16

adc_cali_curve_fitting_config_t cali_config = {
    .unit_id = ADC_UNIT_1,
    .atten = ADC_ATTEN_DB_0,
    .bitwidth = ADC_BITWIDTH_12,
};


adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t calibratie_curve_adc1;

adc_oneshot_unit_init_cfg_t adc_config = {
    .unit_id = ADC_UNIT_1,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
};

adc_oneshot_chan_cfg_t default_channel_config = {
    .bitwidth = ADC_BITWIDTH_12,
    .atten = ADC_ATTEN_DB_0,
};

bool calibratie_ok = false;


void myADC_setup(int channel)
{
    adc_oneshot_new_unit(&adc_config, &adc1_handle);
    adc_oneshot_config_channel(adc1_handle, channel, &default_channel_config);

    #if ADC_CALIBRATION
        if (adc_cali_create_scheme_curve_fitting(&cali_config, &calibratie_curve_adc1) == ESP_OK)
        {
            calibratie_ok = true;
        }
    #endif

}
int myADC_getValue(int channel)
{
    int getal;
    adc_oneshot_read(adc1_handle, channel, &getal);
    return getal;

}
int myADC_getMiliVolt(int channel)
{
    int getal = 0;
    int gemiddelde_raw = 0;
    int voltage = 0;

    for (int i = 0; i < AANTAL_STALEN; i++)
    {
        adc_oneshot_read(adc1_handle, channel, &getal);
        gemiddelde_raw += getal;
    }
    gemiddelde_raw /= AANTAL_STALEN;

    if (calibratie_ok)
    {
        adc_cali_raw_to_voltage(calibratie_curve_adc1, gemiddelde_raw, &voltage);
        return voltage;
    }

    voltage = (gemiddelde_raw * 3300) / 4095;
    return voltage;
}