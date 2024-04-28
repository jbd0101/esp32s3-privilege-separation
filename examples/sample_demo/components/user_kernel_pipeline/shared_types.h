#ifndef SHARED_TYPES_H
enum esp_pipeline_packet_type_t {
    ESP_SYSCALL_EVENT_NONE,
    ESP_SYSCALL_EVENT_TEMP,
    ESP_SYSCALL_EVENT_SENSOR2,
};

typedef struct {
    uint32_t value;
    enum esp_pipeline_packet_type_t type;
} esp_pipeline_packet_t;
#define SHARED_TYPES_H
#endif
