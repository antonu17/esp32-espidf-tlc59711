#include <esp_event.h>
#include <esp_log.h>
#include <math.h>

#include "effects.h"

#define TAG __FILE__

#define SPEED pdMS_TO_TICKS(60)
#define DELAY pdMS_TO_TICKS(180)

static int running;

void sidewaves_stop() {
    ESP_LOGI(TAG, "stop event received");
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, sidewaves_stop));
    running = 0;
}

void sidewaves() {
    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, sidewaves_stop, NULL));
    running = 1;
    float origin_x, origin_y, distance, height, ripple_interval;
    int x, y, i;

    fb_clear();

    while (running) {
        for (i = 0; i < 2000; i++) {
            origin_x = 3.5 + sin((float)i / 500) * 4;
            origin_y = 3.5 + cos((float)i / 500) * 4;

            for (x = 0; x < 8; x++) {
                for (y = 0; y < 8; y++) {
                    distance = distance2d(origin_x, origin_y, x, y) / 9.899495 * 8;
                    ripple_interval = 2;
                    height = 4 + sin(distance / ripple_interval + (float)i / 50) * 3.6;

                    fb_set_pixel(x, y, (int)height, hue_to_rgb_hsv(150 + ((uint8_t)distance * 24)));
                }
            }

            vTaskDelay(2);
            fb_clear();
        }
        vTaskDelay(DELAY);
    }
    fb_clear();

    ESP_LOGD(TAG, "notify effect_loop");
    xTaskNotify(effect_loop_task_handle, 0, eNoAction);
    vTaskDelay(portMAX_DELAY);
}
