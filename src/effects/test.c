#include "effects.h"

#define DELAY pdMS_TO_TICKS(500)

void test(effect_t *effect) {
    fb_clear();
    for (int i = 0; i < 512; i++) {
        frame_buffer[i] = RED;
    }
    vTaskDelay(DELAY);
    for (int i = 0; i < 512; i++) {
        frame_buffer[i] = GREEN;
    }
    vTaskDelay(DELAY);
    for (int i = 0; i < 512; i++) {
        frame_buffer[i] = BLUE;
    }
    vTaskDelay(DELAY);
    fb_clear();
}
