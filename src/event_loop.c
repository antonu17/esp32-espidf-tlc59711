#include "esp_event.h"

#include "event_loop.h"

esp_event_loop_handle_t event_loop;
ESP_EVENT_DEFINE_BASE(MIC_EVENTS);
ESP_EVENT_DEFINE_BASE(EFFECT_EVENTS);

void init_event_loop() {
    esp_event_loop_args_t event_loop_args = {
        .queue_size = 5,
        .task_name = "event_loop_task", // task will be created
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = 2048,
        .task_core_id = tskNO_AFFINITY
    };
    ESP_ERROR_CHECK(esp_event_loop_create(&event_loop_args, &event_loop));
}
