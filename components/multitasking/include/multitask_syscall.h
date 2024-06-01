#pragma once
//create a packet type enum
#include "../../shared/syscall_shared/include/syscall_structs.h"

esp_err_t usr_esp_kernel_start_rr_dispatcher();
esp_err_t usr_esp_kernel_start_preemptive_dispatcher();
esp_err_t usr_esp_kernel_start_deep_sleep_dispatcher();
esp_err_t sys_set_time_before_sleep(uint8_t n_turns,void *callback);

esp_err_t usr_save_task_ctx(usr_task_ctx_t * task_ctx);
esp_err_t usr_get_uint32_secret(char *key, uint32_t *value);
esp_err_t usr_prepare_task_ctx(int N_TASKS,int stack_size);
esp_err_t usr_notify_dispatch();