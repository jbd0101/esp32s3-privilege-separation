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
#include <lwip/sockets.h>
#include <lwip/netdb.h>


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

#define N_TASKS         1
static const char *TAG = "user_main";
static TaskHandle_t * pvTasks;
static usr_task_ctx_t ** taskCtx;
static int counter = 0;

#define CC_U1_SERVER "192.168.1.33"
#define CC_U1_PORT 80

void user_temp(){
    int i = 0;
    esp_pipeline_packet_t packet;
    struct addrinfo *res;

    while(1){
        uint32_t key1;
        usr_get_uint32_secret("key1",&key1);
        ESP_LOGI(TAG,"Hello from user_second %d - secret key1 : %u",i,key1);
        i++;
        //dequeue
        esp_err_t r = usr_esp_kernel_pipeline_receive(&packet);
        if(r == ESP_OK){
            ESP_LOGI(TAG,"Temperature : %d",packet.value);
            //here i can do some processing with the packet and the secret key
            // send it to the server
           // getaddrinfo(CC_U1_SERVER, "80", &hints, &res);
            int s = socket(AF_INET, SOCK_STREAM, 0);
            if (s < 0) {
                ESP_LOGE(TAG, "Failed to allocate socket");
                vTaskDelay(100);
                continue;
            }
            // connect to the ip address
            struct sockaddr_in dest_addr;
            dest_addr.sin_addr.s_addr = inet_addr(CC_U1_SERVER);
            dest_addr.sin_family = AF_INET;
            dest_addr.sin_port = htons(CC_U1_PORT);
            if(connect(s, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in)) != 0) {
                ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);

                close(s);
                vTaskDelay(100);
                continue;
            }
            //create a http/1 request with the temperature value
            char request[100];
            sprintf(request,"POST /index?value=%d HTTP/1.1\r\nHost: %s\r\nUser-Agent: esp-idf/1.0 esp32\r\n\r\n",packet.value,CC_U1_SERVER);
            if (write(s, request, strlen(request)) < 0) {
                ESP_LOGE(TAG, "Write failed");
                close(s);
                vTaskDelay(100);
                continue;
            }
            close(s);


        }else{
            ESP_LOGE(TAG,"Invalid packet type");
        }

        vTaskDelay(700);

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
    int id = counter;
    counter++;
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
    usr_prepare_task_ctx(N_TASKS,2048);
    for (int i = 0; i < N_TASKS; ++i) {
        task_id[i] = i;
       // taskCtx[i] = usr_xTaskCreatePinnedToCoreU(user_generic, "user_generic", 1024, &task_id[i], 1, &pvTasks[i]);
        taskCtx[i] = usr_xTaskCreatePinnedToCoreU(user_temp, "user_second", 2048, &task_id[i], 1, &pvTasks[i]);
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
