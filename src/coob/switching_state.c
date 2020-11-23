#include "switching_state.h"

#include <esp_log.h>
#include <stdlib.h>

#include "coob_state.h"
#include "persistence.h"

void transition_to_switching(coob_state_t state, effect_t *effect, callback_func_t callback) {
    if (NULL == effect) {
        ESP_LOGE(__FILE__, "effect is null");
        return;
    }

    save_effect(effect->name);

    state->current_effect = effect;
    ESP_LOGD(__FILE__, "switched effect: %s", effect->name);

    if (NULL != callback) {
        callback(state);
    }
}
