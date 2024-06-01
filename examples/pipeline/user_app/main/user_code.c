
#include <stdlib.h>
#include "syscall_wrappers.h"
#include "multitask_syscall.h"
#include "pipeline_syscall.h"

#include "esp_log.h"

#define N_TASKS         3
static int counter = 0;
static const char *TAG = "user_main";
static TaskHandle_t *pvTasks;
static usr_task_ctx_t **taskCtx;


void user_app_sensor2and3(void *arg) {
    int id = counter++;
    // subsribe to event sensor
    usr_subscribe_event(ESP_SYSCALL_EVENT_SENSOR2);
    usr_subscribe_event(ESP_SYSCALL_EVENT_SENSOR3);
    while (1) {
        // wait for event
        esp_pipeline_packet_t packet = usr_esp_kernel_pipeline_receive();
        if(packet.type == ESP_SYSCALL_EVENT_SENSOR2){
            ESP_LOGI("APP1_SENSOR_2","Received packet from kernel val: %u",packet.value);
        }else if(packet.type == ESP_SYSCALL_EVENT_SENSOR3){
            ESP_LOGI("APP1_SENSOR_3","Received packet from kernel val: %u",packet.value);
        }
        usr_notify_dispatch();
    }
}

void user_app_sensor2(void *arg) {
    int id = counter++;
    // subsribe to event sensor
    usr_subscribe_event(ESP_SYSCALL_EVENT_SENSOR2);
    while (1) {
        // wait for event
        esp_pipeline_packet_t packet = usr_esp_kernel_pipeline_receive();
        if(packet.type == ESP_SYSCALL_EVENT_SENSOR2){
            ESP_LOGI("APP2_SENSOR_2","Received packet from kernel val: %u",packet.value);
        }
        usr_notify_dispatch();
    }
}
void user_app_no_sensor(void *arg) {
    int id = counter++;
    while (1) {
        usr_notify_dispatch();
    }
}


    void user_main() {
    ESP_LOGI(TAG, "Starting user_main");
    usr_esp_kernel_pipeline_init();
    usr_prepare_task_ctx(N_TASKS, 2048);
    usr_xTaskSchedule(user_app_sensor2and3,"user app sensor 2 and 3");
    usr_xTaskSchedule(user_app_sensor2,"user app sensor 2");
    usr_xTaskSchedule(user_app_no_sensor,"user app no sensor");
    usr_esp_kernel_start_preemptive_dispatcher();
}
