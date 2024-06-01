#include <string.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>

#include <syscall_wrappers.h>
#include <syscall_def.h>
#include <pipeline_syscall.h>
#include "shared_types.h"

#include <freertos/queue.h>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
#endif

esp_err_t usr_esp_kernel_pipeline_init(){
    return EXECUTE_SYSCALL(__NR_esp_kernel_pipeline_init);
}

esp_pipeline_packet_t usr_esp_kernel_pipeline_receive() {
    esp_pipeline_packet_t packet;
    EXECUTE_SYSCALL(&packet, __NR_esp_kernel_pipeline_receive);
    return packet;
}
uint32_t usr_esp_kernel_pipeline_data_waiting(){
    return EXECUTE_SYSCALL(__NR_esp_kernel_pipeline_data_waiting);
}
esp_err_t usr_subscribe_event(uint8_t event){
    return EXECUTE_SYSCALL(event, __NR_esp_subscribe_event);
}



#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif


