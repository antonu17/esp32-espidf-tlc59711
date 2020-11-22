#include "coob.h"

#include <esp_log.h>
#include <stdlib.h>

#include "coob_state.h"
#include "effect_list.h"
#include "loop_state.h"

struct coob {
    struct coob_state state;
};

static coob_t instance = NULL;

static coob_t coob_new(void) {
    coob_t instance = malloc(sizeof *instance);
    if (NULL != instance) {
        instance->state.current_effect = effect_list_first(effect_list);
        /* Specify the initial state. */
        transition_to_loop(&instance->state);
    }
    return instance;
}

static void coob_free(coob_t instance) {
    free(instance);
}

coob_t coob_get_instance(void) {
    if (instance == NULL) {
        instance = coob_new();
    }
    return instance;
}

void coob_mode_loop(coob_t instance) {
    instance->state.loop(&instance->state);
}

void coob_mode_solo(coob_t instance) {
    instance->state.solo(&instance->state);
}

void coob_switch_effect(coob_t instance, int i) {
    instance->state.switch_effect(&instance->state, i);
}

void init_coob() {
    init_effect_list();
    instance = coob_new();
}
