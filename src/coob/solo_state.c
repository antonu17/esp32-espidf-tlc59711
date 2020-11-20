#include "solo_state.h"

#include <esp_log.h>

#include "coob_state.h"

/* Possible transition to the following state: */
#include "loop_state.h"

static void start_loop(coob_state_t state) {
    transition_to_loop(state);
}

void transition_to_solo(coob_state_t state) {
    default_state(state);
    state->loop = start_loop;
    ESP_LOGD(__FILE__, "switched to single mode");
}
