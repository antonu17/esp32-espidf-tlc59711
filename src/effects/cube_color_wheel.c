#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "effects.h"

#define TAG "EFFECT_CUBE_COLOR_WHEEL"

void cube_color_wheel_stop();

void cube_color_wheel() {
    rgb_t rgb;
    uint8_t c = 0;

    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, cube_color_wheel_stop, NULL));

    while (1) {
        rgb = hue_to_rgb_sine2(c);
        for (int px = 0; px < 512; px++) {
            frame_buffer[px] = rgb;
        }
        c++;
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void cube_color_wheel_stop() {
    ESP_LOGI(TAG, "stopped");
    fb_clear();
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, cube_color_wheel_stop));
}
