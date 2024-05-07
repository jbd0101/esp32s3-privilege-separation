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
#include <pipeline_syscall.h>
#include "nvs_flash.h"

#include "wifi_connect.c"
//inclue UART
#include "driver/uart.h"
//temperature
#include "driver/i2c.h"
//builtin temperature sensor
#include "driver/temp_sensor.h"

#include "esp_wifi.h"
#define TAG "protected_app"

temp_sensor_config_t temp_sensor = {
        .dac_offset = TSENS_DAC_L2,
        .clk_div = 6,
};

IRAM_ATTR void user_app_exception_handler(void *arg)
{
    // Perform actions when user app exception happens
}

void app_main()
{
    esp_err_t ret;
    if(nvs_flash_init_partition("static_data") == ESP_OK){
        ESP_LOGI(TAG, "nvs_flash_init_partition success");
    }else{
        ESP_LOGE(TAG, "nvs_flash_init_partition failed");
    }

    //open partition
    nvs_handle_t handler_task1;
    if(nvs_open_from_partition("static_data", "task1", NVS_READWRITE, &handler_task1) == ESP_OK) {
        ESP_LOGI(TAG, "nvs_open_from_partition success for task 1");
    }
    nvs_handle_t handler_task2;

    if(nvs_open_from_partition("static_data", "task2", NVS_READWRITE, &handler_task2) == ESP_OK) {
        ESP_LOGI(TAG, "nvs_open_from_partition success for task 2");
    }


   nvs_set_u32(handler_task1,"key1", 123456);
    nvs_set_u32(handler_task1,"key2", 654321);
    nvs_set_u32(handler_task2,"key1", 20202020);
    nvs_set_u32(handler_task2,"key2", 30303030);

    ret = esp_priv_access_init(user_app_exception_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize PA %d\n", ret);
    }


    esp_priv_access_set_periph_perm(PA_GPIO, PA_WORLD_1, PA_PERM_ALL);
    temp_sensor_set_config(temp_sensor);
    ret = temp_sensor_start();

    sys_esp_kernel_pipeline_init();
    ret = esp_priv_access_user_boot();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to boot user app %d\n", ret);
    }

    wifi_config_t wifi_conf = {
            .sta = {
                    .ssid = "24GhzJC",
                    .password = "15B3BF35A2"
            }
    };

    if(sys_wifi_connect(&wifi_conf) == ESP_OK) {
        ESP_LOGI(TAG, "Connected to wifi");
    }else{
        ESP_LOGE(TAG, "Failed to connect to wifi");
    }


    for (int i = 0; ; i++) {
        //get temperature
        float temp= 10;
        ESP_LOGI(TAG, "Sending Temperature: %f to user apps", temp);
        //push to queue
        esp_pipeline_packet_t packet;
        packet.type = ESP_SYSCALL_EVENT_TEMP;
        packet.value = temp;
        sys_esp_kernel_pipeline_push(packet);

        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}
