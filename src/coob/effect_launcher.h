#if !defined(EFFECT_LAUNCHER_H)
#define EFFECT_LAUNCHER_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "effect.h"

extern TaskHandle_t effect_launcher_task_handle;
extern TaskHandle_t current_effect_task_handle;

void effect_run(effect_t *effect, TickType_t timeout);

#endif  // EFFECT_LAUNCHER_H
