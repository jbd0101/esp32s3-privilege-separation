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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "syscall_wrappers.h"
#include "pipeline_syscall.h"
#include "soc/gpio_struct.h"
#include "hal/gpio_types.h"
#include "driver/gpio.h"
#include "hal/gpio_ll.h"

#include "esp_log.h"
#include "driver/temp_sensor.h"

temp_sensor_config_t temp_sensor = {
        .dac_offset = TSENS_DAC_L2,
        .clk_div = 6,
};

#define BUTTON_IO       0

#define INTR_LED        2
#define BLINK_GPIO      35

#define SYSCALL         0
#define NORMAL          1

#define MODE            SYSCALL
#define MATRIX_SIZE     10
#define DELAY           50

#define N_TASKS         4
static int g_state = 0;
static const char *TAG = "user_main";
static TaskHandle_t * pvTasks;
static usr_task_ctx_t ** taskCtx;



void user_second(){
    int i = 0;
    while(1){
        uint32_t key1;
        usr_get_uint32_secret("key1",&key1);
        ESP_LOGI(TAG,"Hello from user_second %d - secret key1 : %u",i,key1);
        i++;
        vTaskDelay(200);

    }
}

void user_third(){
    int i = 0;
    while(1){
        uint32_t key1;
        usr_get_uint32_secret("key1",&key1);
        ESP_LOGI(TAG,"Hello from user_second %d - secret key1 : %u",i,key1);
        i++;
        vTaskDelay(200);
    }
}

void user_generic(void *arg){
    // get id from args
    int id = *((int *)arg);
    int i = 0;
    while (1) {
        ESP_LOGI(TAG, "Hello from user[%d] - %d", id, i);
        i++;
        vTaskDelay(1000);
    }
}

void user_main()
{
    ESP_LOGI(TAG, "Starting user_main");
    usr_esp_kernel_pipeline_init();

    pvTasks = (TaskHandle_t *)malloc(N_TASKS * sizeof(TaskHandle_t));
    taskCtx = (usr_task_ctx_t **)malloc(N_TASKS * sizeof(usr_task_ctx_t *));

//    TaskHandle_t pvTask1;
//    TaskHandle_t pvTask2;
//
//    int *id1 = (int *)malloc(sizeof(int));
//    int *id2 = (int *)malloc(sizeof(int));

    // create an array of int* to pass the task id, of length N_TASKS
    int *task_id = (int *)calloc(N_TASKS, sizeof(int));
    usr_prepare_task_ctx(N_TASKS,1024);
    for (int i = 0; i < N_TASKS; ++i) {
        task_id[i] = i;
        taskCtx[i] = usr_xTaskCreatePinnedToCoreU(user_generic, "user_generic", 1024, &task_id[i], 1, &pvTasks[i]);
        vTaskSuspend(pvTasks[i]);
        usr_save_task_ctx(taskCtx[i],i);
        if ( taskCtx[i] == NULL) {
            ESP_LOGE(TAG, "Task %d Creation failed", i);
        }
        ESP_LOGI(TAG,"Task %d , taskhandler : %p",i,pvTasks[i]);
        ESP_LOGW(TAG, "Task %d stack size = %d", i, taskCtx[i]->stack_size);

    }
    usr_esp_kernel_start_dispatcher(taskCtx, N_TASKS);
}
