#include "coob_state.h"

#include <esp_log.h>

static void default_idle(coob_state_t state) {
    ESP_LOGD(__FILE__, "transition is not supported in the current mode: %s", state->name);
}

static void default_loop(coob_state_t state) {
    ESP_LOGD(__FILE__, "transition is not supported in the current mode: %s", state->name);
}

static void default_solo(coob_state_t state) {
    ESP_LOGD(__FILE__, "transition is not supported in the current mode: %s", state->name);
}

static void default_switch_effect(coob_state_t state) {
    ESP_LOGD(__FILE__, "switch_effect is not supported in the current mode: %s", state->name);
}

void default_state(coob_state_t state) {
    state->idle = default_idle;
    state->loop = default_loop;
    state->solo = default_solo;
    state->switch_effect = default_switch_effect;
}
