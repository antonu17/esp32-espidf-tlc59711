#include <esp_event.h>
#include <esp_log.h>

#include "effects.h"

#define TAG __FILE__

#define SPEED pdMS_TO_TICKS(100)

static int running;

void rain_stop() {
    ESP_LOGI(TAG, "stop event received");
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, rain_stop));
    running = 0;
}

void rain() {
    rgb_t c = {255, 255, 255};

    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, rain_stop, NULL));
    running = 1;

    fb_clear();
    while (running) {
        int i;
        int rnd_x;
        int rnd_y;
        int rnd_num;

        rnd_num = rand() % 4;

        for (i = 0; i < rnd_num; i++) {
            rnd_x = rand() % 8;
            rnd_y = rand() % 8;
            fb_set_pixel(rnd_x, rnd_y, 7, c);
        }
        vTaskDelay(SPEED);
        fb_shift(FB_AXIS_Z, FB_SHIFT_BACK);
    }
    fb_clear();

    ESP_LOGD(TAG, "notify effect_loop");
    xTaskNotify(effect_loop_task_handle, 0, eNoAction);
    vTaskDelay(portMAX_DELAY);
}
