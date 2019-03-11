/* ADC1 Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

static esp_adc_cal_characteristics_t *adc_chars;
static const adc1_channel_t channel1 = ADC1_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc1_channel_t channel2 = ADC1_CHANNEL_3;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

static void check_efuse()
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

void app_main()
{
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel1, atten);
        adc1_config_channel_atten(channel2, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    printf("raw1,voltage1(mV),pressure1(psi),raw2,voltage2(mV),pressure2(psi)\n");

    //Continuously sample ADC1
    while (1) {
        uint32_t adc_reading1 = 0;
        uint32_t adc_reading2 = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            adc_reading1 += adc1_get_raw(channel1);
            adc_reading2 += adc1_get_raw(channel2);
        }
        adc_reading1 /= NO_OF_SAMPLES;
        adc_reading2 /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage1 = esp_adc_cal_raw_to_voltage(adc_reading1, adc_chars);
        uint32_t voltage2 = esp_adc_cal_raw_to_voltage(adc_reading2, adc_chars);

        //pressure is linear from 0-4.5v with 0.5v=0psi, 2.5v=30psi, 4.5v=60psi
        float psi1 = (voltage1/1000.0-0.5)/4.0 * 60;
        float psi2 = (voltage2/1000.0-0.5)/4.0 * 60;
        printf("%d,%d,%f,%d,%d,%f\n", adc_reading1, voltage1, psi1, adc_reading2, voltage2, psi2);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


