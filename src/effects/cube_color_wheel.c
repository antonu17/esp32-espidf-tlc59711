#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "effects.h"

void cube_color_wheel() {
    rgb_t rgb;
    uint8_t c = 0;

    while (1) {
        rgb = hue_to_rgb_sine2(c);
        for (int px = 0; px < 512; px++) {
            frame_buffer[px] = rgb;
        }
        c++;
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}
