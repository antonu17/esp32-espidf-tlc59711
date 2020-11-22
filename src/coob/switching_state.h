#if !defined(SWITCHING_STATE_H)
#define SWITCHING_STATE_H

#include "coob_state.h"

typedef void (*callback_func_t)(coob_state_t);

void transition_to_switching(coob_state_t state, effect_t *effect, callback_func_t callback);

#endif  // SWITCHING_STATE_H
