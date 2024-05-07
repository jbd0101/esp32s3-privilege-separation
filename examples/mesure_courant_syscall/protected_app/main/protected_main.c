// Copyright 2020-2022 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//         http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/* Privilege Separation Example:
 *
 * This file is for protected application.
 * Developer can write routines which need to execute in protected space
 * In order to start user application, initialize esp_priv_access and call esp_priv_access_user_boot
 */

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_priv_access.h"
#include "ws2812.h"
//temperature
#include "driver/i2c.h"
//builtin temperature sensor
#include "driver/temp_sensor.h"
#define TAG "protected_app"
#define USER_APP 0
#define MODE            SYSCALL
#define MATRIX_SIZE     10
#define DELAY           50
/*
temp_sensor_config_t temp_sensor = {
        .dac_offset = TSENS_DAC_L2,
        .clk_div = 6,
};*/

IRAM_ATTR void user_app_exception_handler(void *arg)
{
    // Perform actions when user app exception happens
}

void app_main()
{
    esp_err_t ret;
#if USER_APP == 1

    ret = esp_priv_access_init(user_app_exception_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize PA %d\n", ret);
    }

    esp_priv_access_set_periph_perm(PA_GPIO, PA_WORLD_1, PA_PERM_ALL);
    //temp_sensor_set_config(temp_sensor);
    //ret = temp_sensor_start();


    ret = esp_priv_access_user_boot();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to boot user app %d\n", ret);
    }

    for (int i = 0; ; i++) {
       ets_printf("Hello from protected environment\n");
       //float temp;
        //temp_sensor_read_celsius(&temp);
        //printf("Temperature: %.2f\n", temp);
       vTaskDelay(5000);
    }
#else
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << 35);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    while (1) {

        int n = 40000000;
        int c=1;
        while(n>0){
            c = c+n;
            n --;
        }
        ESP_LOGI(TAG,"val : %d - %d",n,c);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }



#endif
}
