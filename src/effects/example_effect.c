#include <esp_event.h>
#include <esp_log.h>

#include "effects.h"

#define TAG __FILE__

#define SPEED pdMS_TO_TICKS(60)
#define DELAY pdMS_TO_TICKS(180)

static int running;

void example_effect_stop() {
    ESP_LOGI(TAG, "stop event received");
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, example_effect_stop));
    running = 0;
}

void example_effect() {

    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, example_effect_stop, NULL));
    running = 1;

    fb_clear();
    while (running) {
        vTaskDelay(DELAY);
    }
    fb_clear();

    ESP_LOGD(TAG, "notify effect_loop");
    xTaskNotify(effect_loop_task_handle, 0, eNoAction);
    vTaskDelay(portMAX_DELAY);
}
