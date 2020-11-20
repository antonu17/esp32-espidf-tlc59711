#include <esp_event.h>
#include <esp_log.h>

#include "effect_functions.h"

#define TAG __FILE__

#define SPEED pdMS_TO_TICKS(60)
#define DELAY pdMS_TO_TICKS(180)

void example_effect(effect_t *effect) {

    fb_clear();
    while (effect->running) {
        vTaskDelay(DELAY);
    }
    fb_clear();
}
