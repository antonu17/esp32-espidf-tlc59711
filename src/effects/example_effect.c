#include <esp_event.h>
#include <esp_log.h>

#include "effects.h"

#define TAG "EXAMPLE_EFFECT"

#define SPEED pdMS_TO_TICKS(60)
#define DELAY pdMS_TO_TICKS(180)

void example_effect_stop();

void example_effect() {
    uint8_t end = 1;
    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, example_effect_stop, NULL));
    fb_clear();
    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    fb_clear();
    xQueueSend(effects_queue, (void*)&end, (TickType_t)0);
    vTaskDelete(NULL);
}

void example_effect_stop() {
    fb_clear();
    ESP_LOGI(TAG, "stopped");
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, example_effect_stop));
}
