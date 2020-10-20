#include <esp_event.h>
#include <esp_log.h>

#include "effects.h"

#define TAG "EFFECT_RAIN"

void rain_stop();

void rain() {
    rgb_t c = {255, 255, 255};

    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, rain_stop, NULL));

    fb_clear();
    while (1) {
        int i;
        int rnd_x;
        int rnd_y;
        int rnd_num;

        rnd_num = rand() % 4;

        for (i = 0; i < rnd_num; i++) {
            rnd_x = rand() % 8;
            rnd_y = rand() % 8;
            fb_set_pixel(rnd_x, rnd_y, 0, c);
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
        fb_shift(FB_AXIS_Z, FB_SHIFT_FORWARD);
    }
}

void rain_stop() {
    ESP_LOGI(TAG, "stopped");
    fb_clear();
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, rain_stop));
}
