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

static int g_state = 0;
//static usr_gpio_handle_t intr_gpio_handle;

static const char *TAG = "user_main";

/*
 * matrice of nxn sum function , where n is the size of the matrix a[
 */
int *usr_syscall_mat_sum(int *a,int *b,int n)
{
    int *c = malloc(sizeof(int)*n*n);
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            c[i*n+j] = usr_add_a_b(a[i*n+j],b[i*n+j]);
        }
    }
    return c;
}

int *usr_normal_mat_sum(int *a,int *b,int n)
{
    int *c = malloc(sizeof(int)*n*n);
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            c[i*n+j] = a[i*n+j]+b[i*n+j];
        }
    }
    return c;
}
/* User space code is never executed in ISR context,
 * so user registered "ISR" functions are actually executed
 * from task's context, hence they are termed as softISRs
 */
UIRAM_ATTR void user_gpio_softisr(void *arg)
{
    int gpio_num = (int)arg;
    if (g_state == 1) {
        gpio_ll_set_level(&GPIO, gpio_num, 0);
        g_state = 0;
    } else {
        gpio_ll_set_level(&GPIO, gpio_num, 1);
        g_state = 1;
    }
}

void blink_task()
{


    while (1) {
        ESP_LOGI(TAG, "Blinking LED");
        gpio_ll_set_level(&GPIO, BLINK_GPIO, 1);
        vTaskDelay(100);
        int a = 1;
        int b = 2;
        int c = usr_add_a_b(a, b);
        ESP_LOGI(TAG, "Sum of %d and %d is %d", a, b, c);

        gpio_ll_set_level(&GPIO, BLINK_GPIO, 0);
        vTaskDelay(100);
        int temp = usr_get_internal_temperature();
        ESP_LOGI(TAG, "Temperature: %d \n", temp);
        vTaskDelay(100);
        int *a1 = malloc(sizeof(int)*4);
        int *b1 = malloc(sizeof(int)*4);
        for(int i=0;i<4;i++){
            a1[i] = i;
            b1[i] = i;
        }
        int *c1 = usr_normal_mat_sum(a1,b1,2);
        for(int i=0;i<4;i++){
            ESP_LOGI(TAG, "Sum of %d and %d is %d", a1[i], b1[i], c1[i]);
        }
        int *c2 = usr_syscall_mat_sum(a1,b1,2);
        for(int i=0;i<4;i++){
            ESP_LOGI(TAG, "Sum of %d and %d is %d", a1[i], b1[i], c2[i]);
        }
        free(a1);
        free(b1);
        free(c1);
        free(c2);

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
    if (xTaskCreate(blink_task, "Blink task", 4096, NULL, 1, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Task Creation failed");
    }
}
