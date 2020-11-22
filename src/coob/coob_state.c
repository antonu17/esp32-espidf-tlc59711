#include "coob_state.h"

#include <esp_log.h>

static void default_loop() {
    ESP_LOGD(__FILE__, "transition is not supported in the current mode");
}

static void default_solo() {
    ESP_LOGD(__FILE__, "transition is not supported in the current mode");
}

void default_state(coob_state_t state) {
    state->loop = default_loop;
    state->solo = default_solo;
}
