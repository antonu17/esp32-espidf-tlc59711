#include <esp_event.h>
#include <esp_log.h>

#include "effects.h"

#define TAG __FILE__

void shift_planes_stop() {
    ESP_LOGI(TAG, "stop event received");
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, shift_planes_stop));
}

void shift_planes() {
    rgb_t c = {255, 255, 255};
    int i, plane;

    // ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, shift_planes_stop, NULL));

    fb_clear();

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
    fb_clear();

    ESP_LOGD(TAG, "notify effect_loop");
    xTaskNotify(effect_loop_task_handle, 0, eNoAction);
    vTaskDelay(portMAX_DELAY);
}
