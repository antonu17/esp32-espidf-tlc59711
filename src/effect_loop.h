#if !defined(EFFECT_LOOP_H)
#define EFFECT_LOOP_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "effect_list.h"

void effect_run(effect_t *effect);
void init_effects();

#endif  // EFFECT_LOOP_H
