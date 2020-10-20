#include <esp_event.h>
#include <esp_log.h>

#include "effects.h"

#define TAG "EXAMPLE_EFFECT"

void example_effect_stop();

void example_effect() {
    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, example_effect_stop, NULL));
    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void example_effect_stop() {
    ESP_LOGI(TAG, "stopped");
    fb_clear();
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, example_effect_stop));
}
