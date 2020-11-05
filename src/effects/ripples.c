#include <esp_event.h>
#include <esp_log.h>
#include <math.h>

#include "effects.h"

#define TAG __FILE__

#define SPEED pdMS_TO_TICKS(10)
#define DELAY pdMS_TO_TICKS(180)
#define STEP 10

static int running;

void ripples_stop() {
    ESP_LOGI(TAG, "stop event received");
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, ripples_stop));
    running = 0;
}

void ripples() {
    float distance, height, ripple_interval;
    int x, y;
    uint16_t i = 0;
    uint8_t step = STEP;
    uint8_t c = 150;

    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, ripples_stop, NULL));
    running = 1;

    while (running) {
        i++;

        fb_clear_draft();
        for (x = 0; x < 8; x++) {
            for (y = 0; y < 8; y++) {
                distance = distance2d(3.5, 3.5, x, y);
                ripple_interval = 1.3;
                height = 4 + sin(distance / ripple_interval + (float)i / 50) * 4;
                fb_set_pixel_draft(x, y, (int)height, hue_to_rgb_hsv(c + ((uint8_t)distance * 15)));
            }
        }
        fb_draw_draft();

        if(!step--) {
            step = STEP;
            c++;
        }
        vTaskDelay(1);
    }

    ESP_LOGI(TAG, "notify effect_loop");
    xTaskNotify(effect_loop_task_handle, 0, eNoAction);
    vTaskDelay(portMAX_DELAY);
}
