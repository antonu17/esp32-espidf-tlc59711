#if !defined(COOB_STATE_H)
#define COOB_STATE_H

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "effect.h"
#include "coob.h"

typedef struct coob_state* coob_state_t;

typedef void (*switch_mode_func_t)(coob_state_t);
typedef coob_err_t (*switch_effect_func_t)(coob_state_t, char* effect_name);

typedef enum {
    LOOP = 0,
    SOLO = 1,
    IDLE = 2,
} coob_mode_t;

struct coob_state {
    char* name;
    effect_t* current_effect;
    coob_mode_t current_mode;
    switch_mode_func_t idle;
    switch_mode_func_t loop;
    switch_mode_func_t solo;
    switch_effect_func_t switch_effect;
};

void default_state(coob_state_t state);

#endif  // COOB_STATE_H
