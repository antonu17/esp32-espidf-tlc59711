#if !defined(COOB_STATE_H)
#define COOB_STATE_H
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "effect.h"

typedef struct coob_state* coob_state_t;

typedef void (*effect_loop_func_t)(coob_state_t);
typedef void (*effect_solo_func_t)(coob_state_t);

struct coob_state {
    char* name;
    effect_t *current_effect;
    effect_loop_func_t loop;
    effect_solo_func_t solo;
};

void default_state(coob_state_t state);

#endif  // COOB_STATE_H
