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

#pragma once
//create a packet type enum
#include "../shared_types.h"
#include "syscall_structs.h"

esp_err_t usr_esp_kernel_pipeline_init();
esp_pipeline_packet_t usr_esp_kernel_pipeline_receive();
uint32_t usr_esp_kernel_pipeline_data_waiting();
esp_err_t sys_esp_kernel_pipeline_push(esp_pipeline_packet_t data);
esp_err_t usr_esp_kernel_start_dispatcher(usr_task_ctx_t** taskCtx, int n_tasks);
esp_err_t usr_save_task_ctx(usr_task_ctx_t * task_ctx);
esp_err_t usr_get_uint32_secret(char *key, uint32_t *value);