#include <esp_event.h>
#include <esp_log.h>

#include "effects.h"

#define TAG __FILE__

void shift_planes(effect_t *effect) {
    rgb_t c = {255, 255, 255};
    int i, plane;

    fb_clear();
    for (plane = 0; plane < 3; plane++) {
        for (i = 0; i < 8; i++) {
            fb_clear();
            fb_set_plane(plane, i, c);
            vTaskDelay(70 / portTICK_PERIOD_MS);
        }
        for (i = 7; i >= 0; i--) {
            fb_clear();
            fb_set_plane(plane, i, c);
            vTaskDelay(70 / portTICK_PERIOD_MS);
        }
    }
    fb_clear();
}
