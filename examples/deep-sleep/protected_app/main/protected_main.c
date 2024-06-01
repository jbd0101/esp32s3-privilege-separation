#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_priv_access.h"
#include <multitask_syscall.h>
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "driver/rtc_io.h"


#define TAG "protected_app"
#define DEEP_SLEEP_WAKEUP_TIME_S 20

static void deep_sleep_register_rtc_timer_wakeup(void)
{
    ets_printf("Enabling timer wakeup, %ds\n", DEEP_SLEEP_WAKEUP_TIME_S);
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(DEEP_SLEEP_WAKEUP_TIME_S * 1000000));
}
IRAM_ATTR void user_app_exception_handler(void *arg)
{
    // Perform actions when user app exception happens
}
static void *callback_end_of_tasks(){
    ets_printf("End of tasks\n");
    nvs_handle_t handler_task1;

    if(nvs_open_from_partition("static_data", "ehc", NVS_READWRITE, &handler_task1) == ESP_OK) {
        ESP_LOGI(TAG, "nvs_open_from_partition success for task 1");
    }
    nvs_set_i16(handler_task1,"last_task", -1);
    esp_deep_sleep_start();

}

void app_main() {
    sys_set_time_before_sleep(2, callback_end_of_tasks);

    esp_err_t ret;
    ret = esp_priv_access_init(user_app_exception_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize PA %d\n", ret);
    }
    if(nvs_flash_init_partition("static_data") == ESP_OK){
        ESP_LOGI(TAG, "nvs_flash_init_partition success");
    }else{
        ESP_LOGE(TAG, "nvs_flash_init_partition failed");
    }
    sys_set_time_before_sleep(10, callback_end_of_tasks);


    esp_priv_access_set_periph_perm(PA_GPIO, PA_WORLD_1, PA_PERM_ALL);

    ret = esp_priv_access_user_boot();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to boot user app %d\n", ret);
    }

    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_TIMER: {
            ets_printf("Wake up from timer. \n");
            break;
        }
        default:
            ets_printf("Wake up from other sources\n");
            break;
    }
    rtc_gpio_isolate(GPIO_NUM_12);
    deep_sleep_register_rtc_timer_wakeup();

    for (int i = 0;; i++) {
        ets_printf("Hello from protected environment\n");
        vTaskDelay(500);
    }
}
