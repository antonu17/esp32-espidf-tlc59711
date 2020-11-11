#include <esp_event.h>
#include <esp_log.h>

#include "effects.h"

#define TAG __FILE__

#define SPEED pdMS_TO_TICKS(40)
#define DELAY pdMS_TO_TICKS(120)

void wireframe_woop_corner(uint8_t x, uint8_t y, uint8_t z, rgb_t c) {
    uint8_t x2, y2, z2;
    for (int i = 0; i < 8; i++) {
        if (x > 0) {
            x2 = 7 - i;
        } else {
            x2 = i;
        }
        if (y > 0) {
            y2 = 7 - i;
        } else {
            y2 = i;
        }
        if (z > 0) {
            z2 = 7 - i;
        } else {
            z2 = i;
        }
        fb_clear();
        fb_draw_wireframe(x, y, z, x2, y2, z2, c);
        vTaskDelay(SPEED);
    }
    vTaskDelay(DELAY);
    for (int i = 7; i >= 0; i--) {
        if (x > 0) {
            x2 = 7 - i;
        } else {
            x2 = i;
        }
        if (y > 0) {
            y2 = 7 - i;
        } else {
            y2 = i;
        }
        if (z > 0) {
            z2 = 7 - i;
        } else {
            z2 = i;
        }
        fb_clear();
        fb_draw_wireframe(x, y, z, x2, y2, z2, c);
        vTaskDelay(SPEED);
    }
    vTaskDelay(DELAY);
}

void wireframe_woop_center(rgb_t c) {
    int i;

    for (i = 0; i < 4; i++) {
        fb_clear();
        fb_draw_wireframe(4 + i, 4 + i, 4 + i, 3 - i, 3 - i, 3 - i, c);
        vTaskDelay(SPEED * 2);
    }
    vTaskDelay(DELAY);
    for (i = 3; i >= 0; i--) {
        fb_clear();
        fb_draw_wireframe(4 + i, 4 + i, 4 + i, 3 - i, 3 - i, 3 - i, c);
        vTaskDelay(SPEED * 2);
    }
    vTaskDelay(DELAY);
}

void wireframes(effect_t *effect) {

    fb_clear();
    while (effect->running) {
        wireframe_woop_corner(0, 0, 0, RED);
        wireframe_woop_corner(0, 0, 7, GREEN);
        wireframe_woop_corner(0, 7, 0, BLUE);
        wireframe_woop_corner(0, 7, 7, RED);
        wireframe_woop_corner(7, 0, 0, GREEN);
        wireframe_woop_corner(7, 0, 7, BLUE);
        wireframe_woop_corner(7, 7, 0, RED);
        wireframe_woop_corner(7, 7, 7, GREEN);
        wireframe_woop_center(BLUE);
        wireframe_woop_center(RED);
        wireframe_woop_center(GREEN);
        wireframe_woop_center(BLUE);
    }
    fb_clear();
}
