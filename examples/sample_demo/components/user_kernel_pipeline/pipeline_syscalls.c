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
#include "nvs_flash.h"

//#include <soc_defs.h>
#include "soc_defs.h"

#define true 1
#define false 0

#include <esp_log.h>
#include <pipeline_syscall_priv.h>
#include <freertos/queue.h>
#include "syscall_structs.h"

#include <esp_map.h>
#define TASK_STACK_SIZE 2048
static TaskHandle_t *pvTasks;
static usr_task_ctx_t *tskCtxs;
//static StackType_t * sleeping_task_stack1;
//static StackType_t * sleeping_task_stack2;
static const char *TAG = "pipeline_syscalls";
static int task_index = 0;
static QueueHandle_t sys_kernel_pipeline_queue = NULL;

static int n_tasks = 0;
static StackType_t *sleeping_task_stacks;

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
        return ESP_FAIL;
    }
    if(xQueueReceive(sys_kernel_pipeline_queue, packet, 0) != pdTRUE){
        ESP_LOGE(TAG, "Failed to receive data from sys_kernel_pipeline_queue");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Received packet with value: %u", packet->value);

    return ESP_OK;
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

TaskHandle_t sys_get_task_handle(TaskHandle_t xTask){
    int wrapper_index = (int)xTask;
    esp_map_handle_t *wrapper_handle = esp_map_verify(wrapper_index, ESP_MAP_TASK_ID);
    if (wrapper_handle == NULL) {
        return;
    }
    return (TaskHandle_t)wrapper_handle->handle;

}

// tache1, tache2, et tache 3 => stack_user
// kernel stack_kernel_temporaire
void sys_user_tasks_dispatcher(){
    ESP_LOGW(TAG, "Starting user dispatcher");
    task_index = 0;
    //restore the stack of index0
    memcpy(tskCtxs[0].stack, &sleeping_task_stacks[0*TASK_STACK_SIZE], tskCtxs[0].stack_size);
    while(1){
        sys_vTaskResume2(pvTasks[task_index]);
        vTaskDelay(2000);
        sys_vTaskSuspend2(pvTasks[task_index]);
        ESP_LOGI(TAG, "Suspending task %p", pvTasks[task_index]);
        int next = (task_index + 1) % n_tasks;
        int current = task_index;
        if (memcpy(&sleeping_task_stacks[current*TASK_STACK_SIZE],tskCtxs[current].stack, tskCtxs[current].stack_size) == NULL) {
            ESP_LOGE(TAG, "Failed to copy stack of task %d to buffer", current);
        } else {
            ESP_LOGI(TAG, "Copied stack of task %d to buffer, current stack address = %p ", current, &sleeping_task_stacks[current*tskCtxs[current].stack_size]);
        }
        if (memcpy(tskCtxs[next].stack, &sleeping_task_stacks[next*TASK_STACK_SIZE], tskCtxs[next].stack_size) == NULL) {
            ESP_LOGE(TAG, "Failed to copy buffer to stack of task %d", next);
        } else {
            ESP_LOGI(TAG, "Copied buffer to stack of task %d, next stack address = %p", next, &sleeping_task_stacks[next*1024]);
        }
        // print new index, new stack_size and new stack address used
        ESP_LOGI(TAG, "Task %d | stack size = %d", next, tskCtxs[next].stack_size);

        task_index = next;

    }



}
esp_err_t sys_esp_kernel_start_dispatcher(usr_task_ctx_t** taskCtx, int n) {
    n_tasks = n;
    xTaskCreatePinnedToCore(sys_user_tasks_dispatcher, "sys_user_tasks_dispatcher", 2048, NULL, 5, NULL, 1);
    return ESP_OK;
}

esp_err_t sys_save_task_ctx(usr_task_ctx_t *task_ctx, int index){
    tskCtxs[index] = *task_ctx;
    pvTasks[index] = *((TaskHandle_t *)(task_ctx->task_handle));
    ESP_LOGI(TAG, "SAVING Task %d | stack size = %d", index, tskCtxs[index].stack_size);
    void *p = sleeping_task_stacks;
    p += index*1024;
    //memcpy to the sleeping_task_stack
    memcpy(p, task_ctx->stack, task_ctx->stack_size);
    return true;
}
esp_err_t sys_get_uint32_secret(char *key, uint32_t *value){
    //
    nvs_handle_t handler;
    if(task_index == 0){
        if(nvs_open_from_partition("static_data", "task1", NVS_READWRITE, &handler) != ESP_OK){
            ESP_LOGE(TAG, "Failed to open nvs partition");
            return ESP_FAIL;
        }
    }else{
        if(nvs_open_from_partition("static_data", "task2", NVS_READWRITE, &handler) != ESP_OK){
            ESP_LOGE(TAG, "Failed to open nvs partition");
            return ESP_FAIL;
        }
    }
    if(nvs_get_u32(handler, key, value) != ESP_OK){
        ESP_LOGE(TAG, "Failed to get value from nvs");
        return ESP_FAIL;
    }
    //close nvs
    nvs_close(handler);
    return ESP_OK;
}
esp_err_t sys_prepare_task_ctx(int N_TASKS,int stack_size){
    n_tasks = N_TASKS;
    ESP_LOGI(TAG, "Preparing task context for %d tasks", N_TASKS);
    pvTasks = (TaskHandle_t *)heap_caps_malloc(n_tasks * sizeof(TaskHandle_t), MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);
    tskCtxs = (usr_task_ctx_t *) heap_caps_malloc(n_tasks * sizeof(usr_task_ctx_t), MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);
    sleeping_task_stacks = (StackType_t *)heap_caps_malloc(stack_size*N_TASKS, MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);

    if (!sleeping_task_stacks) {
        ESP_LOGE(TAG, "Failed to allocate memory for sleeping_task_stacks");
        return ESP_FAIL;
    }
    return ESP_OK;
}
