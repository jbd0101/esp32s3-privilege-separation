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

#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <soc_defs.h>
#define true 1
#define false 0

#include <esp_log.h>
#include <pipeline_syscall_priv.h>
#include <freertos/queue.h>

#include <esp_map.h>


static const char *TAG = "pipeline_syscalls";

static QueueHandle_t sys_kernel_pipeline_queue = NULL;
esp_err_t sys_esp_kernel_pipeline_init(){
    if(sys_kernel_pipeline_queue == NULL){
        sys_kernel_pipeline_queue = xQueueCreate(10, sizeof(uint32_t));
        if(sys_kernel_pipeline_queue == NULL){
            ESP_LOGE(TAG, "Failed to create sys_kernel_pipeline_queue");
            return ESP_FAIL;
        }
    }else{
        ESP_LOGW(TAG, "sys_kernel_pipeline_queue already created");
    }
    return ESP_OK;
}
uint32_t sys_esp_kernel_pipeline_receive(){
    uint32_t data;
    if(sys_kernel_pipeline_queue == NULL){
        ESP_LOGE(TAG, "sys_kernel_pipeline_queue not created");
        return 0;
    }
    if(xQueueReceive(sys_kernel_pipeline_queue, &data, portMAX_DELAY) != pdTRUE){
        ESP_LOGE(TAG, "Failed to receive data from sys_kernel_pipeline_queue");
        return 0;
    }
    return data;
}

uint32_t sys_esp_kernel_pipeline_data_waiting(){
    if(sys_kernel_pipeline_queue == NULL){
        ESP_LOGE(TAG, "sys_kernel_pipeline_queue not created");
        return false;
    }
    return uxQueueMessagesWaiting(sys_kernel_pipeline_queue);
}
esp_err_t sys_esp_kernel_pipeline_push(uint32_t data){
    if(sys_kernel_pipeline_queue == NULL){
        ESP_LOGE(TAG, "sys_kernel_pipeline_queue not created");
        return ESP_FAIL;
    }
    if(xQueueSend(sys_kernel_pipeline_queue, &data, 0) != pdTRUE){
        ESP_LOGE(TAG, "Failed to send data to sys_kernel_pipeline_queue");
        return ESP_FAIL;
    }
    return ESP_OK;
}