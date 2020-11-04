#include <esp_event.h>
#include <esp_log.h>
#include <math.h>

#include "effects.h"

#define TAG __FILE__

#define SPEED pdMS_TO_TICKS(10)
#define DELAY pdMS_TO_TICKS(180)

static int running;

void ripples_stop() {
    ESP_LOGI(TAG, "stop event received");
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, ripples_stop));
    running = 0;
}

float distance2d(float x1, float y1, float x2, float y2) {
    float dist;
    dist = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));

    return dist;
}

void ripples() {
    float distance, height, ripple_interval;
    int x, y;
    uint8_t i = 0;

    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, ripples_stop, NULL));
    running = 1;

    while (running) {
        fb_clear();
        for (x = 0; x < 8; x++) {
            for (y = 0; y < 8; y++) {
                distance = distance2d(3.5, 3.5, x, y);
                ripple_interval = 1.3;
                height = 4 + sin(distance / ripple_interval + (float)i / 50) * 4;
                fb_set_pixel(x, y, (int)height, hue_to_rgb_hsv(150 + ((uint8_t)distance * 24)));
            }
        }
        i++;
        vTaskDelay(2);
    }

    ESP_LOGI(TAG, "notify effect_loop");
    xTaskNotify(effect_loop_task_handle, 0, eNoAction);
    vTaskDelay(portMAX_DELAY);
}

// (3.5 - 3.5) * (3.5 - 3.5) + (0 - 0) * (0 - 0)
// (3.5 - 3.5) * (3.5 - 3.5) + (8 - 0) * (8 - 0)
// (3.5 - 3.5) * (3.5 - 3.5) + (0 - 8) * (0 - 8)
// (3.5 - 3.5) * (3.5 - 3.5) + (2 - 8) * (2 - 8)
