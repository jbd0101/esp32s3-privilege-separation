
#include <stdlib.h>
#include "syscall_wrappers.h"
#include "multitask_syscall.h"
#include "esp_log.h"

#define N_TASKS         3
static int counter = 0;
static const char *TAG = "user_main";
static TaskHandle_t *pvTasks;
static usr_task_ctx_t **taskCtx;


void user_generic(void *arg) {
    int id = counter++;

    int n = (id + 1) * 1e6;
    int cnt = 1;
    uint32_t sum = 0;
    while (1) {
        sum = 0;
        for (int i = 0; i < n; i++) {
            sum = sum + i;
        }
        ESP_LOGI("USER_GENERIC", "Hello from %d\t| it = %d\t| sum =  %u", id, cnt, sum);
        cnt++;
        usr_notify_dispatch();
    }
}

void user_main() {
    ESP_LOGI(TAG, "Starting user_main");

    usr_prepare_task_ctx(N_TASKS, 2048);
    for (int i = 0; i < N_TASKS; ++i) {
        usr_xTaskSchedule(user_generic,"user_generic");
    }

    usr_esp_kernel_start_preemptive_dispatcher();
}
