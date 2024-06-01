
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_priv_access.h"
#include <multitask_syscall.h>
#include <pipeline_syscall.h>

#define TAG "protected_app"


IRAM_ATTR void user_app_exception_handler(void *arg) {
    // Perform actions when user app exception happens
}

void app_main() {
    esp_err_t ret;

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
        esp_pipeline_packet_t packet;
        packet.type = ESP_SYSCALL_EVENT_SENSOR2;
        packet.value = 222;
        sys_esp_kernel_pipeline_push(packet);
        packet.type = ESP_SYSCALL_EVENT_SENSOR3;
        packet.value = 333;
        sys_esp_kernel_pipeline_push(packet);
        vTaskDelay(500/ portTICK_PERIOD_MS);
    }
}
