#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "effect.h"

#include <esp_log.h>
#include <stdlib.h>

effect_t *effect_new(char *name, effect_function_t function, stop_hook_t stop_hook) {
    effect_t *effect = malloc(sizeof *effect);
    if (effect) {
        effect->name = name;
        effect->function = function;
        effect->stop_hook = stop_hook;
        ESP_LOGD(__FILE__, "create effect: %s\t\t(%p)", name, effect);
    }
    return effect;
}

void effect_free(effect_t *effect) {
    ESP_LOGD(__FILE__, "delete effect: %s\t\t(%p)", effect->name, effect);
    free(effect);
}
