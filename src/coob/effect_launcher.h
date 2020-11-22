#if !defined(EFFECT_LAUNCHER_H)
#define EFFECT_LAUNCHER_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "effect.h"

void effect_run(effect_t *effect, TickType_t timeout);
void effect_terminate(effect_t *effect);

#endif  // EFFECT_LAUNCHER_H
