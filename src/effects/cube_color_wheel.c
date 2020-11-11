#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "effects.h"

#define TAG __FILE__

#define SPEED pdMS_TO_TICKS(20)
#define DELAY pdMS_TO_TICKS(180)

void cube_color_wheel(effect_t *effect) {
    rgb_t rgb;
    uint8_t c = 0;

    fb_clear();
    while (effect->running) {
        rgb = hue_to_rgb_sine2(c);
        for (int px = 0; px < 512; px++) {
            frame_buffer[px] = rgb;
        }
        c++;
        vTaskDelay(SPEED);
    }
    fb_clear();
}
