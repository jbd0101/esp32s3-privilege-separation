

#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include "soc_defs.h"

#define true 1
#define false 0
#include <esp_log.h>
#include "shared_types.h"
#include <freertos/queue.h>
#include "syscall_structs.h"
#include <esp_map.h>

 static int pipeline_task_index = 0;
 static int pipeline_n_task = 0;
static const char *PIP_TAG = "PIPELINE";
static QueueHandle_t sys_kernel_pipeline_queue = NULL;

static QueueHandle_t * users_queues = NULL;
static uint8_t **subscribed_events =NULL;

esp_err_t set_pipeline_current_task_index(int index){
    pipeline_task_index = index;
    return ESP_OK;
}
esp_err_t init_pipeline_queues(int N_TASKS){
    pipeline_n_task = N_TASKS;
    users_queues = (QueueHandle_t *)heap_caps_malloc(N_TASKS * sizeof(QueueHandle_t), MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);
    subscribed_events = (uint8_t **)heap_caps_malloc(N_TASKS * sizeof(uint8_t *), MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);
    for(int i = 0; i < pipeline_n_task; i++){
        subscribed_events[i] = (uint8_t *)heap_caps_malloc(1 * sizeof(uint8_t), MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);
        subscribed_events[i][0] = 0;
        users_queues[i] = xQueueCreate(10, sizeof(esp_pipeline_packet_t));
    }

    return ESP_OK;
}

esp_err_t sys_esp_kernel_pipeline_init(){
    if(sys_kernel_pipeline_queue == NULL){
        sys_kernel_pipeline_queue = xQueueCreate(10, sizeof(esp_pipeline_packet_t));
        if(sys_kernel_pipeline_queue == NULL){
            ESP_LOGE(PIP_TAG, "Failed to create sys_kernel_pipeline_queue");
            return ESP_FAIL;
        }
    }else{
        ESP_LOGW(PIP_TAG, "sys_kernel_pipeline_queue already created");
    }
    return ESP_OK;
}

QueueHandle_t sys_get_current_user_queue(){
    return users_queues[pipeline_task_index];
}
esp_err_t sys_esp_kernel_pipeline_receive(esp_pipeline_packet_t *packet){
    if(!is_valid_user_d_addr((void *) packet)){
        ESP_LOGE(PIP_TAG, "packet address is not in user space");
        return false;
    }
    if(xQueueReceive(sys_get_current_user_queue(), packet, 0) != pdTRUE){
        return ESP_FAIL;
    }
    ESP_LOGI(PIP_TAG, "Received packet with value: %u", packet->value);


    return ESP_OK;
}

uint32_t sys_esp_kernel_pipeline_data_waiting(){

    return uxQueueMessagesWaiting(sys_get_current_user_queue());
}
esp_err_t sys_esp_kernel_pipeline_push(esp_pipeline_packet_t data){
    if(sys_kernel_pipeline_queue == NULL){
        ESP_LOGE(PIP_TAG, "sys_kernel_pipeline_queue not created");
        return ESP_FAIL;
    }
    for(int i = 0; i < pipeline_n_task; i++){
        for(int j = 1; j <= subscribed_events[i][0]; j++){
            if(subscribed_events[i][j] == data.type){
                if(xQueueSend(users_queues[i], &data, 0) != pdTRUE){
                    ESP_LOGE(PIP_TAG, "Queue of task %d is full ", i);
                }
            }
        }

    }
    return ESP_OK;
}
esp_err_t sys_subscribe_event(uint8_t event){
    subscribed_events[pipeline_task_index][0] += 1;
    subscribed_events[pipeline_task_index] = (uint8_t *)heap_caps_realloc(subscribed_events[pipeline_task_index], subscribed_events[pipeline_task_index][0] * sizeof(uint8_t), MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);
    subscribed_events[pipeline_task_index][subscribed_events[pipeline_task_index][0]] = event;
    return ESP_OK;
}
