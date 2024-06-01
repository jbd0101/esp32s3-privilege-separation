
#include <syscall_wrappers.h>
#include <syscall_def.h>
#include <multitask_syscall.h>


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
#endif


esp_err_t usr_esp_kernel_start_rr_dispatcher() {
    return EXECUTE_SYSCALL(__NR_esp_kernel_start_rr_dispatcher);
}

esp_err_t usr_esp_kernel_start_preemptive_dispatcher() {
    return EXECUTE_SYSCALL(__NR_esp_kernel_start_preemptive_dispatcher);
}

esp_err_t usr_esp_kernel_start_deep_sleep_dispatcher() {
    return EXECUTE_SYSCALL(__NR_esp_kernel_start_deep_sleep_dispatcher);
}

esp_err_t usr_save_task_ctx(usr_task_ctx_t *task_ctx) {
    return EXECUTE_SYSCALL(task_ctx, __NR_esp_save_task_ctx);
}

esp_err_t usr_get_uint32_secret(char *key, uint32_t *value) {
    return EXECUTE_SYSCALL(key, value, __NR_esp_get_uint32_secret);
}

esp_err_t usr_prepare_task_ctx(int N_TASKS, int stack_size) {
    return EXECUTE_SYSCALL(N_TASKS, stack_size, __NR_esp_prepare_task_ctx);
}


esp_err_t usr_notify_dispatch() {
    vTaskDelay(1);
    return EXECUTE_SYSCALL(__NR_esp_notify_dispatch);
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif


