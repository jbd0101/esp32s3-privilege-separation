
#include <stdlib.h>
#include "syscall_wrappers.h"
#include "multitask_syscall.h"
#include "esp_log.h"

#define N_TASKS         2
static int counter = 0;
static const char *TAG = "user_main";


void user_generic(void *arg) {
    int id = counter++;
    int cnt = 1;
    uint32_t key1 = 0;
    uint32_t key2 = 0;
    while (1) {
        usr_get_uint32_secret("key1", &key1);
        usr_get_uint32_secret("key2", &key2);
        ESP_LOGI("USER_GENERIC", "Hello from %d\t| it = %d\t| key1 =  %u\t| key2 = %u", id, cnt, key1, key2);
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
