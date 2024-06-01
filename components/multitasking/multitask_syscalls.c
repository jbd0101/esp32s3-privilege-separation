#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include "nvs_flash.h"
#include "../pipeline/include/pipeline_syscall.h"

#include "../shared/esp_priv_build_utils/include/soc_defs.h"

#define true 1
#define false 0

#define ROUND_ROBIN 0
#define PREEMPTIVE 1

#define TIMEOUT 2000 //max execution time for user tasks [ms]
#include <esp_log.h>
#include <freertos/queue.h>
#include "../shared/syscall_shared/include/syscall_structs.h"
#include <esp_map.h>

static TaskHandle_t *pvTasks;


static usr_task_ctx_t *tskCtxs;
char *TAG = "multitasking_syscalls";
static uint16_t STACK_SIZE = 2048;
static int task_index = 0;

static int n_tasks = 0;
static uint8_t created_task_i = 0;

static StackType_t *sleeping_task_stacks;
static TaskHandle_t dispatchTask = NULL;

static void (*call_back_n_turns)();
static uint8_t n_turns_before_sleep = 0;
static nvs_handle_t nvs_error_handler;
static uint32_t tasks_mask= ~0;


void resume_task(TaskHandle_t xTaskToResume) {
    int wrapper_index = (int) xTaskToResume;
    esp_map_handle_t *wrapper_handle = esp_map_verify(wrapper_index, ESP_MAP_TASK_ID);
    if (wrapper_handle == NULL) {
        return;
    }
    vTaskResume((TaskHandle_t) wrapper_handle->handle);
}
void set_last_task_nvs(int last_task_index){
    if(nvs_set_i16(nvs_error_handler, "last_task", (int16_t) last_task_index) != ESP_OK){
        ESP_LOGE(TAG, "Failed to set last_task in nvs");
    }
}
void reset_all_ehc(){
    tasks_mask = ~0;
    if(nvs_set_u32(nvs_error_handler, "tasks_mask", tasks_mask) != ESP_OK){
        ESP_LOGE(TAG, "Failed to set tasks_mask in nvs");
    }
    set_last_task_nvs(-1);
}
void suspend_task(TaskHandle_t xTaskToSuspend) {
    if (xTaskToSuspend == NULL) {
        return vTaskSuspend(NULL);
    }
    int wrapper_index = (int) xTaskToSuspend;
    esp_map_handle_t *wrapper_handle = esp_map_verify(wrapper_index, ESP_MAP_TASK_ID);
    if (wrapper_handle == NULL) {
        return;
    }
    vTaskSuspend((TaskHandle_t) wrapper_handle->handle);
}

TaskHandle_t sys_get_task_handle(TaskHandle_t xTask) {
    int wrapper_index = (int) xTask;
    esp_map_handle_t *wrapper_handle = esp_map_verify(wrapper_index, ESP_MAP_TASK_ID);
    if (wrapper_handle == NULL) {
        return NULL;
    }
    return (TaskHandle_t) wrapper_handle->handle;

}

void sys_user_tasks_dispatcher(void *arg) {
    int mode = (int) arg;
    if (mode == ROUND_ROBIN) {
        ESP_LOGI(TAG, "Starting round robin dispatcher");
    } else if (mode == PREEMPTIVE) {
        ESP_LOGI(TAG, "Starting preemptive dispatcher");
        if (dispatchTask == NULL) {
            dispatchTask = xTaskGetCurrentTaskHandle();
        } else {
            ESP_LOGE(TAG, "Dispatcher already created");
        }
    } else {
        ESP_LOGE(TAG, "Invalid mode");
    }

    task_index = 0;
    memcpy(tskCtxs[0].stack, &sleeping_task_stacks[0 * STACK_SIZE], tskCtxs[0].stack_size);
    while (1) {
        set_pipeline_current_task_index(task_index);
        resume_task(pvTasks[task_index]);
        if (mode == ROUND_ROBIN) {
            vTaskDelay(pdMS_TO_TICKS(TIMEOUT));
        } else {
            ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(TIMEOUT));
        }
        suspend_task(pvTasks[task_index]);
        set_pipeline_current_task_index(task_index);

        int next = (task_index + 1) % n_tasks;
        int current = task_index;
        if (memcpy(&sleeping_task_stacks[current * STACK_SIZE], tskCtxs[current].stack, tskCtxs[current].stack_size) ==
            NULL) {
            ESP_LOGE(TAG, "Failed to copy stack of task %d to buffer", current);
        }
        if (memcpy(tskCtxs[next].stack, &sleeping_task_stacks[next * STACK_SIZE], tskCtxs[next].stack_size) == NULL) {
            ESP_LOGE(TAG, "Failed to copy buffer to stack of task %d", next);
        }
        task_index = next;
    }

}

void sys_user_deep_sleep_dispatcher(void *arg) {
    ESP_LOGI(TAG, "Starting deep sleep dispatcher");
    if (dispatchTask == NULL) {
        dispatchTask = xTaskGetCurrentTaskHandle();
    } else {
        ESP_LOGE(TAG, "Dispatcher already created");
    }

    task_index = 0;
    memcpy(tskCtxs[0].stack, &sleeping_task_stacks[0 * STACK_SIZE], tskCtxs[0].stack_size);
    while (1) {
        set_last_task_nvs(task_index);
        set_pipeline_current_task_index(task_index);
        resume_task(pvTasks[task_index]);
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(TIMEOUT));
        suspend_task(pvTasks[task_index]);
        n_turns_before_sleep--;
        if (n_turns_before_sleep == 0) {
            call_back_n_turns();
        }
        int next = (task_index + 1) % n_tasks;
        while ((tasks_mask & (1 << next)) == 0) {
            next = (next + 1) % n_tasks;
        }
        set_pipeline_current_task_index(next);

        int current = task_index;
        if (memcpy(&sleeping_task_stacks[current * STACK_SIZE], tskCtxs[current].stack, tskCtxs[current].stack_size) ==
            NULL) {
            ESP_LOGE(TAG, "Failed to copy stack of task %d to buffer", current);
        }
        if (memcpy(tskCtxs[next].stack, &sleeping_task_stacks[next * STACK_SIZE], tskCtxs[next].stack_size) == NULL) {
            ESP_LOGE(TAG, "Failed to copy buffer to stack of task %d", next);
        }
        task_index = next;
    }

}

esp_err_t sys_esp_kernel_start_rr_dispatcher() {
    if(created_task_i != n_tasks){
        ESP_LOGE(TAG, "Not all tasks have been created");
        return ESP_FAIL;
    }
    int i = ROUND_ROBIN;
    xTaskCreatePinnedToCore(sys_user_tasks_dispatcher, "sys_user_tasks_dispatcher", STACK_SIZE, (void*) i, 5, NULL, 1);
    return ESP_OK;
}

esp_err_t sys_esp_kernel_start_preemptive_dispatcher() {
    if(created_task_i != n_tasks){
        ESP_LOGE(TAG, "Not all tasks have been created");
        return ESP_FAIL;
    }
    int i = PREEMPTIVE;
    xTaskCreatePinnedToCore(sys_user_tasks_dispatcher, "sys_user_tasks_dispatcher", STACK_SIZE, (void*)i, 5, NULL, 1);
    return ESP_OK;
}

esp_err_t sys_esp_kernel_start_deep_sleep_dispatcher() {
    if(created_task_i != n_tasks){
        ESP_LOGE(TAG, "Not all tasks have been created");
        return ESP_FAIL;
    }
    n_turns_before_sleep *=n_tasks;
    if(nvs_open_from_partition("static_data", "ehc", NVS_READWRITE, &nvs_error_handler) != ESP_OK){
        ESP_LOGE(TAG, "Failed to open nvs partition");
    }
    if(nvs_get_u32(nvs_error_handler, "tasks_mask", &tasks_mask) != ESP_OK){
        tasks_mask = ~0;
        if(nvs_set_u32(nvs_error_handler, "tasks_mask", tasks_mask) != ESP_OK){
            ESP_LOGE(TAG, "Failed to set tasks_mask in nvs");
        }
    }

    int16_t last_task_index;
    if(nvs_get_i16(nvs_error_handler, "last_task", &last_task_index) != ESP_OK){
        last_task_index = -1;
        set_last_task_nvs(last_task_index);
    }else{
        if(last_task_index != -1){
            ESP_LOGE(TAG, "Last task index is not -1, something went wrong at task %d", last_task_index);
            tasks_mask = tasks_mask & ~(1 << last_task_index);
            if(nvs_set_u32(nvs_error_handler, "tasks_mask", tasks_mask) != ESP_OK){
                ESP_LOGE(TAG, "Failed to set tasks_mask in nvs");
            }
        }
    }
    xTaskCreatePinnedToCore(sys_user_deep_sleep_dispatcher, "sys_user_tasks_dispatcher", STACK_SIZE, NULL, 5, NULL, 1);
    return ESP_OK;
}

esp_err_t sys_save_task_ctx(usr_task_ctx_t *task_ctx) {
    int index = created_task_i;
    if(index >= n_tasks){
        ESP_LOGE(TAG, "Failed to save task context, no more space");
        return false;
    }
    tskCtxs[index] = *task_ctx;
    pvTasks[index] = *((TaskHandle_t * )(task_ctx->task_handle));
    ESP_LOGI(TAG, "SAVING Task %d | stack size = %d", index, tskCtxs[index].stack_size);
    void *p = sleeping_task_stacks;
    p += index * STACK_SIZE;
    created_task_i++;

    memcpy(p, task_ctx->stack, task_ctx->stack_size);
    return true;
}

esp_err_t sys_get_uint32_secret(char *key, uint32_t *value) {
    nvs_handle_t handler;
    if (task_index == 0) {
        if (nvs_open_from_partition("static_data", "task1", NVS_READWRITE, &handler) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to open nvs partition");
            return ESP_FAIL;
        }
    } else {
        if (nvs_open_from_partition("static_data", "task2", NVS_READWRITE, &handler) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to open nvs partition");
            return ESP_FAIL;
        }
    }
    if (nvs_get_u32(handler, key, value) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get value from nvs");
        return ESP_FAIL;
    }
    nvs_close(handler);
    return ESP_OK;
}

esp_err_t sys_prepare_task_ctx(int N_TASKS, int stack_size) {
    n_tasks = N_TASKS;
    STACK_SIZE = stack_size;
    ESP_LOGI(TAG, "Preparing task context for %d tasks", N_TASKS);
    pvTasks = (TaskHandle_t *) heap_caps_malloc(n_tasks * sizeof(TaskHandle_t),
                                                MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);
    tskCtxs = (usr_task_ctx_t *) heap_caps_malloc(n_tasks * sizeof(usr_task_ctx_t),
                                                  MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);
    sleeping_task_stacks = (StackType_t *) heap_caps_malloc(stack_size * N_TASKS,
                                                            MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);
    init_pipeline_queues(n_tasks);

    if (!sleeping_task_stacks) {
        ESP_LOGE(TAG, "Failed to allocate memory for sleeping_task_stacks");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t sys_notify_dispatch() {
    if (dispatchTask == NULL) {
        ESP_LOGE(TAG, "Unable to notify dispatcher, task handle is NULL");
        return ESP_FAIL;
    } else {
        xTaskNotifyGive(dispatchTask);
        return ESP_OK;
    }
}


esp_err_t sys_set_time_before_sleep(uint8_t n,void *callback){
    if(call_back_n_turns != NULL){
        ESP_LOGE(TAG, "Time before sleep already set");
        return ESP_FAIL;
    }
    call_back_n_turns = callback;
    n_turns_before_sleep = n;
    return ESP_OK;
}
