#include <esp_event.h>
#include <esp_log.h>

#include "effects.h"

#define TAG "EFFECT_SHIFT_PLANES"

void shift_planes_stop();

void shift_planes() {
    rgb_t c = {255, 255, 255};
    int i, plane;
    uint8_t end = 1;
    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, shift_planes_stop, NULL));

    for (int count = 0; count < 3; count++) {
        for (plane = 0; plane < 3; plane++) {
            for (i = 0; i < 8; i++) {
                fb_clear();
                fb_set_plane(plane, i, c);
                vTaskDelay(70 / portTICK_PERIOD_MS);
            }
            for (i = 7; i >= 0; i--) {
                fb_clear();
                fb_set_plane(plane, i, c);
                vTaskDelay(70 / portTICK_PERIOD_MS);
            }
        }
    }
    xQueueSend(effects_queue, (void*)&end, (TickType_t)0);
    vTaskDelete(NULL);
}

void shift_planes_stop() {
    ESP_LOGI(TAG, "stopped");
    fb_clear();
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, shift_planes_stop));
}
