/* -- includes ----------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "lis3dh.h"

#define TASK_STACK_DEPTH 2048

#define I2C_BUS       0
#define I2C_SCL_PIN   22
#define I2C_SDA_PIN   23
#define I2C_FREQ      I2C_FREQ_400K

#define DEFAULT_VREF    1130        //Use adc2_vref_to_gpio() to obtain a better estimate

static esp_adc_cal_characteristics_t *adc_chars;
static const adc1_channel_t channel1 = ADC1_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc1_channel_t channel2 = ADC1_CHANNEL_0;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_2_5;//0-1.10v
static const adc_unit_t unit = ADC_UNIT_1;

static lis3dh_sensor_t* sensor;

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

// uint32_t get_vref()
// {
//     adc2_vref_to_gpio(25);

//     uint32_t reading = 0;
//     for (int i = 0; i < NO_OF_SAMPLES; i++) {
//         reading += adc1_get_raw(ADC2_CHANNEL_8);
//     }
//     reading /= NO_OF_SAMPLES;

//     //Convert adc_reading to voltage in mV
//     return esp_adc_cal_raw_to_voltage(reading, adc_chars);
// }

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

void read_data ()
{
    lis3dh_float_data_t data;

    uint32_t adc_reading1 = 0;
    uint32_t adc_reading2 = 0;
    uint32_t voltage1;
    uint32_t voltage2;
    float psi1;
    float psi2;

    adc_reading1 = adc1_get_raw(channel1);
    adc_reading2 = adc1_get_raw(channel2);

    voltage1 = esp_adc_cal_raw_to_voltage(adc_reading1, adc_chars);
    voltage2 = esp_adc_cal_raw_to_voltage(adc_reading2, adc_chars);

    psi1 = (voltage1 / 1000.0 - 0.515) / 4.0 * 60;
    psi2 = (voltage2 / 1000.0 - 0.527) / 4.0 * 60;
    

    if (lis3dh_new_data(sensor) &&
        lis3dh_get_float_data(sensor, &data))
    {
        printf("%.3f,%+7.3f,%+7.3f,%+7.3f,%f,%f\n",
               (double)sdk_system_get_time() * 1e-3, data.ax, data.ay, data.az, psi1, psi2);
    }
}

/*
 * In this example, user task fetches the sensor values every seconds.
 */

void user_task_periodic(void *pvParameters)
{
    while (1)
    {
        // read sensor data
        read_data();
    }
}

/* -- main program ------------------------------------------------- */

void user_init(void)
{
    uart_set_baudrate(I2C_BUS,2000000);
    // Give the UART some time to settle
    vTaskDelay(1);

    /** -- MANDATORY PART -- */

    // init all I2C bus interfaces at which LIS3DH  sensors are connected
    i2c_init (I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ);
    
    // init the sensor with slave address LIS3DH_I2C_ADDRESS_1 connected to I2C_BUS.
    sensor = lis3dh_init_sensor (I2C_BUS, LIS3DH_I2C_ADDRESS_1, 0);
    
    if (sensor)
    {
        // configure HPF and reset the reference by dummy read
        lis3dh_config_hpf (sensor, lis3dh_hpf_normal, 0, true, true, true, true);
        lis3dh_get_hpf_ref (sensor);
        
        // enable ADC inputs and temperature sensor for ADC input 3
        lis3dh_enable_adc (sensor, true, true);
        
        // LAST STEP: Finally set scale and mode to start measurements
        lis3dh_set_scale(sensor, lis3dh_scale_2_g);
        lis3dh_set_mode (sensor, lis3dh_odr_5000, lis3dh_normal, true, true, true);

        /** -- TASK CREATION PART --- */

        // must be done last to avoid concurrency situations with the sensor
        // configuration part

        // create a user task that fetches data from sensor periodically
        xTaskCreate(user_task_periodic, "user_task_periodic", TASK_STACK_DEPTH, NULL, 12, NULL);


    }
    else
        printf("Could not initialize LIS3DH sensor\n");
}

void app_main()
{
    check_efuse();

    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel1, atten);
        adc1_config_channel_atten(channel2, atten);
        adc2_config_channel_atten(ADC2_CHANNEL_8, atten);
    }

    adc2_vref_to_gpio(27);
    //uint32_t vref = get_vref();

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    user_init();

    printf("raw1,voltage1(mV),pressure1(psi),raw2,voltage2(mV),pressure2(psi)\n");
}

