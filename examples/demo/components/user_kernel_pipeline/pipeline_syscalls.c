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
//#include <soc_defs.h>
#include "soc_defs.h"

#define true 1
#define false 0

#include <esp_log.h>
#include <pipeline_syscall_priv.h>
#include <freertos/queue.h>
#include "syscall_structs.h"

#include <esp_map.h>

static TaskHandle_t *pvTasks;
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
esp_err_t sys_esp_kernel_pipeline_receive(esp_pipeline_packet_t *packet){
    ESP_LOGI(TAG, "looking for a packet");
    if(!is_valid_user_d_addr((void *) packet)){
        ESP_LOGE(TAG, "packet address is not in user space");
        return false;
    }
    if(xQueueReceive(sys_kernel_pipeline_queue, packet, 0) != pdTRUE){
        ESP_LOGE(TAG, "Failed to receive data from sys_kernel_pipeline_queue");
        return;
    }
    ESP_LOGI(TAG, "Received packet with value: %u", packet->value);

    //store buffer in a0,a1,a2,a3,a4
  /*  asm volatile (
        "addi a1, %0,0\n"
        "addi a2, %1,0\n"
        "addi a3, %2,0\n"
        "addi a4, %3,0\n"
        "addi a5, %4,0\n"
        :
        : "r" (buffer[0]), "r" (buffer[1]), "r" (buffer[2]), "r" (buffer[3]), "r" (buffer[4])
    );*/
    return 1;
}

uint32_t sys_esp_kernel_pipeline_data_waiting(){
    if(sys_kernel_pipeline_queue == NULL){
        ESP_LOGE(TAG, "sys_kernel_pipeline_queue not created");
        return false;
    }
    return uxQueueMessagesWaiting(sys_kernel_pipeline_queue);
}
esp_err_t sys_esp_kernel_pipeline_push(esp_pipeline_packet_t data){
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


void sys_vTaskResume2(TaskHandle_t xTaskToResume)
{
    int wrapper_index = (int)xTaskToResume;
    esp_map_handle_t *wrapper_handle = esp_map_verify(wrapper_index, ESP_MAP_TASK_ID);
    if (wrapper_handle == NULL) {
        return;
    }
    vTaskResume((TaskHandle_t)wrapper_handle->handle);
}

void sys_vTaskSuspend2(TaskHandle_t xTaskToSuspend)
{
    if (xTaskToSuspend == NULL) {
        return vTaskSuspend(NULL);
    }
    int wrapper_index = (int)xTaskToSuspend;
    esp_map_handle_t *wrapper_handle = esp_map_verify(wrapper_index, ESP_MAP_TASK_ID);
    if (wrapper_handle == NULL) {
        return;
    }
    vTaskSuspend((TaskHandle_t)wrapper_handle->handle);
}
void sys_user_tasks_dispatcher(){
    int current_task = 0;
    //suspendall
    for(int i = 1; i < 2; i++) {
        sys_vTaskSuspend2(pvTasks[i]);
    }

    while(1){
        //suspend previous task
        sys_vTaskSuspend2(pvTasks[current_task]);

        //suspendall
        ESP_LOGI(TAG, "Hello from user_dispatch_task %d", current_task);
        current_task = (current_task + 1) % 2;
        sys_vTaskResume2(pvTasks[current_task]);
        vTaskDelay(2000);
    }
}
esp_err_t sys_esp_kernel_start_dispatcher(usr_task_ctx_t* taskCtx1, usr_task_ctx_t* taskCtx2) {
    TaskHandle_t *pvTasks = (TaskHandle_t *)heap_caps_malloc(2 * sizeof(TaskHandle_t), MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);
    if (!pvTasks) {
        return ESP_FAIL;
    }

    ESP_LOGW(TAG, "Task 1 stack size = %d", taskCtx1->stack_size);
    ESP_LOGW(TAG, "Task 2 stack size = %d", taskCtx2->stack_size);
    pvTasks[0] = *((TaskHandle_t *)(taskCtx1->task_handle));
    pvTasks[1] = *((TaskHandle_t *)(taskCtx2->task_handle));

    ESP_LOGW("TEST", "Task 1 stack size = %d, address of task handle = %p | pvtask = %p", taskCtx1->stack_size, (void *)taskCtx1->task_handle, (void *)pvTasks[0]);
    ESP_LOGW("TEST", "Task 2 stack size = %d, address of task handle = %p | pvtask = %p", taskCtx2->stack_size, (void *)taskCtx2->task_handle, (void *)pvTasks[1]);

    xTaskCreatePinnedToCore(sys_user_tasks_dispatcher, "sys_user_tasks_dispatcher", 2048, NULL, 5, NULL, 1);

    return ESP_OK;
}



