
#pragma once
//create a packet type enum
#include "../shared_types.h"
#include "../../shared/syscall_shared/include/syscall_structs.h"

esp_err_t usr_esp_kernel_pipeline_init();
esp_pipeline_packet_t usr_esp_kernel_pipeline_receive();
uint32_t usr_esp_kernel_pipeline_data_waiting();
esp_err_t sys_esp_kernel_pipeline_push(esp_pipeline_packet_t data);
esp_err_t usr_subscribe_event(uint8_t event);
esp_err_t set_pipeline_current_task_index(int index);
esp_err_t init_pipeline_queues(int N_TASKS);
