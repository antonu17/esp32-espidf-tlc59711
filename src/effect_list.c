#include "effect_list.h"

#include <esp_log.h>
#include <stdlib.h>
#include "effects/effects.h"

effect_list_t *effect_list;

effect_t *effect_new(char *name, effect_function_t function, stop_hook_t stop_hook) {
    effect_t *effect = malloc(sizeof(effect_t));
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

effect_list_t *effect_list_new() {
    effect_list_t *list = malloc(sizeof(effect_list_t));
    list->len = 0;
    list->cap = 10;
    list->effect = malloc(10 * sizeof(effect_t));
    return list;
}

effect_list_t *effect_list_add(effect_list_t *effects, effect_t *effect) {
    if (!effects) {
        return effects;
    }
    unsigned int cap = effects->cap;
    unsigned int len = effects->len;
    if (len == cap) {
        cap *= 2;
        effect_t *src = (effect_t *)effects->effect;
        effect_t *dst = malloc(cap * sizeof(effect_t));
        for (unsigned int i = 0; i < len; i++) {
            dst[i] = src[i];
        }
        free(src);
        effects->effect = (effect_t **)dst;
        effects->cap = cap;
    }
    effects->effect[len] = effect;
    effects->len += 1;
    return effects;
}

void init_effect_list() {
    effect_list = effect_list_new();
    effect_list = effect_list_add(effect_list, effect_new("fireworks", fireworks, NULL));
    effect_list = effect_list_add(effect_list, effect_new("ball", ball, NULL));
    effect_list = effect_list_add(effect_list, effect_new("linespin", linespin, NULL));
    effect_list = effect_list_add(effect_list, effect_new("shift_planes", shift_planes, NULL));
    effect_list = effect_list_add(effect_list, effect_new("ripples", ripples, NULL));
    effect_list = effect_list_add(effect_list, effect_new("sinelines", sinelines, NULL));
    effect_list = effect_list_add(effect_list, effect_new("shift_suspend", shift_suspend, NULL));
    effect_list = effect_list_add(effect_list, effect_new("sidewaves", sidewaves, NULL));
    effect_list = effect_list_add(effect_list, effect_new("mic_spectrum", mic_spectrum, mic_spectrum_stop));
    effect_list = effect_list_add(effect_list, effect_new("cube_color_wheel", cube_color_wheel, NULL));
    effect_list = effect_list_add(effect_list, effect_new("vertical_travelers", vertical_travelers, NULL));
    effect_list = effect_list_add(effect_list, effect_new("wireframes", wireframes, NULL));
    effect_list = effect_list_add(effect_list, effect_new("rain", rain, NULL));
    effect_list = effect_list_add(effect_list, effect_new("wave_color_wheel", wave_color_wheel, NULL));

    ESP_LOGI(__FILE__, "Total number of effects: %d", effect_list->len);
}
