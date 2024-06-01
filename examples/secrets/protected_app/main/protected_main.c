
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_priv_access.h"
#include "nvs_flash.h"
#include <multitask_syscall.h>

#define TAG "protected_app"


IRAM_ATTR void user_app_exception_handler(void *arg) {
    // Perform actions when user app exception happens
}

void app_main() {
    esp_err_t ret;
    if(nvs_flash_init_partition("static_data") == ESP_OK){
        ESP_LOGI(TAG, "nvs_flash_init_partition success");
    }else{
        ESP_LOGE(TAG, "nvs_flash_init_partition failed");
    }

    nvs_handle_t handler_task1;
    nvs_handle_t handler_task2;

    if(nvs_open_from_partition("static_data", "task1", NVS_READWRITE, &handler_task1) != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open_from_partition failed for task 1");
    }
    if(nvs_open_from_partition("static_data", "task2", NVS_READWRITE, &handler_task2) != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open_from_partition failed for task 2");
    }

    nvs_set_u32(handler_task1,"key1", 123456);
    nvs_set_u32(handler_task1,"key2", 654321);
    nvs_set_u32(handler_task2,"key1", 20202020);
    nvs_set_u32(handler_task2,"key2", 30303030);


    ret = esp_priv_access_init(user_app_exception_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize PA %d\n", ret);
    }

    esp_priv_access_set_periph_perm(PA_GPIO, PA_WORLD_1, PA_PERM_ALL);

    ret = esp_priv_access_user_boot();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to boot user app %d\n", ret);
    }

    for (int i = 0;; i++) {
        ets_printf("Hello from protected environment\n");
        vTaskDelay(500);
    }
}
