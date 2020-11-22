#include "coob.h"

#include <stdlib.h>

#include "coob_state.h"
#include "loop_state.h"
#include "effect_list.h"

struct coob {
    struct coob_state state;
};

static coob_t instance = NULL;

static coob_t coob_new(void) {
    coob_t instance = malloc(sizeof *instance);
    if (NULL != instance) {
        /* Specify the initial state. */
        instance->state.current_effect = effect_list_first(effect_list);
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

void init_coob() {
    init_effect_list();
    instance = coob_new();
}
