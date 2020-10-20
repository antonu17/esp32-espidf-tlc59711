#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "effects.h"

#define TAG "EFFECT_WAVE_COLOR_WHEEL"

void wave_color_wheel_stop();

void wave_color_wheel() {
    rgb_t rgb;
    uint8_t c0 = 0, c1 = 10, c2 = 20, c3 = 30, c4 = 40, c5 = 50, c6 = 60, c7 = 70;

    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, wave_color_wheel_stop, NULL));

    while (1) {
        for (int row = 0; row < 8; row++) {
            switch (row) {
                case 0:
                    rgb = hue_to_rgb_linear(c0);
                    break;
                case 1:
                    rgb = hue_to_rgb_linear(c1);
                    break;
                case 2:
                    rgb = hue_to_rgb_linear(c2);
                    break;
                case 3:
                    rgb = hue_to_rgb_linear(c3);
                    break;
                case 4:
                    rgb = hue_to_rgb_linear(c4);
                    break;
                case 5:
                    rgb = hue_to_rgb_linear(c5);
                    break;
                case 6:
                    rgb = hue_to_rgb_linear(c6);
                    break;
                case 7:
                    rgb = hue_to_rgb_linear(c7);
                    break;
            }

            for (int x = 0; x < 64; x++) {
                frame_buffer[row * 64 + x] = rgb;
            }
        }

        c0--;
        c1--;
        c2--;
        c3--;
        c4--;
        c5--;
        c6--;
        c7--;

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void wave_color_wheel_stop() {
    ESP_LOGI(TAG, "stopped");
    fb_clear();
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, wave_color_wheel_stop));
}
