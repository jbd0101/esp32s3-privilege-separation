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


static int g_state = 0;
//static usr_gpio_handle_t intr_gpio_handle;

static const char *TAG = "user_main";
static TaskHandle_t * pvTasks;
/* User space code is never executed in ISR context,
 * so user registered "ISR" functions are actually executed
 * from task's context, hence they are termed as softISRs
 */
//UIRAM_ATTR void user_gpio_softisr(void *arg)
//{
//    int gpio_num = (int)arg;
//    if (g_state == 1) {
//        gpio_ll_set_level(&GPIO, gpio_num, 0);
//        g_state = 0;
//    } else {
//        gpio_ll_set_level(&GPIO, gpio_num, 1);
//        g_state = 1;
//    }
//}

void blink_task()
{


    while (1) {
        uint32_t t = usr_esp_log_early_timestamp();
        ESP_LOGI(TAG,"time : %u",t);
        uint32_t eles = usr_esp_kernel_pipeline_data_waiting();
        ESP_LOGI(TAG,"elements : %u",eles);
        esp_pipeline_packet_t packet = usr_esp_kernel_pipeline_receive();
        ESP_LOGI(TAG,"packet : %u",packet.value);
        vTaskDelay(1000);
    }
}

void user_second(){
    while(1){
        ESP_LOGI(TAG,"Hello from user_second");
        vTaskDelay(1000);

    }
}

void user_third(){
    while(1){
        ESP_LOGI(TAG,"Hello from user_third");
        vTaskDelay(1000);
    }
}
void user_main()
{
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << 35);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    usr_esp_kernel_pipeline_init();
    pvTasks = (TaskHandle_t *)malloc(2 * sizeof(TaskHandle_t));
    usr_start_internal_temperature(&temp_sensor);
    /*
    io_conf.pin_bit_mask = (1 << INTR_LED);
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1 << BUTTON_IO);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_softisr_handler_add(BUTTON_IO, user_gpio_softisr, (void*)INTR_LED, &intr_gpio_handle);
    */

    TaskHandle_t pvTask1;
    TaskHandle_t pvTask2;

    usr_task_ctx_t *taskCtx1 = usr_xTaskCreatePinnedToCoreU(user_second, "user second", 1024, NULL, 1, &pvTask1);
    usr_task_ctx_t *taskCtx2 = usr_xTaskCreatePinnedToCoreU(user_third, "user third", 2048, NULL, 1, &pvTask2);

    if ( taskCtx1 == NULL) {
        ESP_LOGE(TAG, "Task Creation failed");
    }
    if (taskCtx2 == NULL) {
        ESP_LOGE(TAG, "Task Creation failed");
    }
    ESP_LOGW(TAG, "Task 1 stack size = %d", taskCtx1->stack_size);
    ESP_LOGW(TAG, "Task 2 stack size = %d", taskCtx2->stack_size);
    usr_esp_kernel_start_dispatcher(pvTask1,pvTask2);
    //vTaskSuspend(pvTask);
}
