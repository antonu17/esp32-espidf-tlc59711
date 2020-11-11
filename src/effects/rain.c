#include <esp_event.h>
#include <esp_log.h>

#include "effects.h"

#define TAG __FILE__

#define SPEED pdMS_TO_TICKS(100)

void rain(effect_t *effect) {
    rgb_t c = {255, 255, 255};

    fb_clear();
    while (effect->running) {
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
}
