#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "effects.h"

void wave_color_wheel() {
    rgb_t rgb;
    uint8_t c0 = 0, c1 = 10, c2 = 20, c3 = 30, c4 = 40, c5 = 50, c6 = 60, c7 = 70;
    while (1) {
        for (int row = 0; row < 8; row++) {
            switch (row) {
                case 0:
                    rgb = hue_to_rgb_sine2(c0);
                    break;
                case 1:
                    rgb = hue_to_rgb_sine2(c1);
                    break;
                case 2:
                    rgb = hue_to_rgb_sine2(c2);
                    break;
                case 3:
                    rgb = hue_to_rgb_sine2(c3);
                    break;
                case 4:
                    rgb = hue_to_rgb_sine2(c4);
                    break;
                case 5:
                    rgb = hue_to_rgb_sine2(c5);
                    break;
                case 6:
                    rgb = hue_to_rgb_sine2(c6);
                    break;
                case 7:
                    rgb = hue_to_rgb_sine2(c7);
                    break;
            }

            for (int x = 0; x < 64; x++) {
                frame_buffer[row * 64 + x] = rgb;
            }
        }

        c0++;
        c1++;
        c2++;
        c3++;
        c4++;
        c5++;
        c6++;
        c7++;

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}
