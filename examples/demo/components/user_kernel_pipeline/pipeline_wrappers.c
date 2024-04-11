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

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>

#include <syscall_wrappers.h>
#include <syscall_def.h>
#include <pipeline_syscall.h>
#include <pipeline_syscall_priv.h>

//include QueueHandle_t
#include <freertos/queue.h>
#define RMAKER_DISPATCHER_TASK_PRIO 22

static const char *TAG = "rmaker_syscalls";

static QueueHandle_t usr_rmaker_dispatcher_queue;
static TaskHandle_t usr_rmaker_dispatcher_task_handle;



#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
#endif
/// icic

esp_err_t usr_esp_kernel_pipeline_init(){
    return EXECUTE_SYSCALL(__NR_esp_kernel_pipeline_init);
}

esp_pipeline_packet_t usr_esp_kernel_pipeline_receive() {
    esp_pipeline_packet_t packet;
    EXECUTE_SYSCALL(&packet, __NR_esp_kernel_pipeline_receive);
    return packet;
}
uint32_t usr_esp_kernel_pipeline_data_waiting(){
    return EXECUTE_SYSCALL(__NR_esp_kernel_pipeline_data_waiting);
}

esp_err_t usr_esp_kernel_start_dispatcher(usr_task_ctx_t* taskCtx1, usr_task_ctx_t* taskCtx2){
    return EXECUTE_SYSCALL(taskCtx1,taskCtx2, __NR_esp_kernel_start_dispatcher);
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif


