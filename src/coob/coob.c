#include "coob.h"

#include <esp_log.h>
#include <stdlib.h>

#include "coob_state.h"
#include "effect_list.h"
#include "loop_state.h"
#include "persistence.h"
#include "solo_state.h"
#include "storage.h"

struct coob {
    struct coob_state state;
};

static coob_t instance = NULL;

static coob_t coob_new(void) {
    coob_t instance = malloc(sizeof *instance);
    if (NULL != instance) {
        /* Specify the initial effect */
        char *effect_name = load_effect();
        effect_t *effect = effect_list_get_by_name(effect_list, effect_name);
        if (NULL == effect) {
            effect = effect_list_first(effect_list);
        }
        instance->state.current_effect = effect;

        /* Specify the initial mode */
        coob_mode_t mode = load_mode();
        switch (mode) {
            case LOOP:
                transition_to_loop(&instance->state);
                break;
            case SOLO:
                transition_to_solo(&instance->state);
                break;

            default:
                break;
        }
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

void coob_switch_effect(coob_t instance, char *effect_name) {
    instance->state.switch_effect(&instance->state, effect_name);
}

int coob_get_mode(coob_t instance) {
    return instance->state.current_mode;
}

char* coob_get_effect(coob_t instance) {
    return instance->state.current_effect->name;
}

void init_coob() {
    init_storage();
    init_effect_list();
    instance = coob_new();
}
