#if !defined(_CUBE_EFFECTS_H_)
#define _CUBE_EFFECTS_H_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "event_loop.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct effect_t effect_t;
typedef struct effect_list_t effect_list_t;

typedef void (*stop_hook_t)(effect_t *effect);

typedef struct effect_t {
    const char *name;
    TaskFunction_t function;
    stop_hook_t stop_hook;
    int running;
} effect_t;

typedef struct effect_list_t {
    effect_t **effect;
    unsigned int len;
    unsigned int cap;
} effect_list_t;

effect_list_t *effect_list_new();
effect_list_t *effect_list_add(effect_list_t *effects, effect_t *effect);

effect_t *effect_new(char *name, TaskFunction_t function, stop_hook_t stop_hook);
void effect_free(effect_t *effect);
void effect_run(effect_t *effect);
void init_effects();

#ifdef __cplusplus
}
#endif

#endif  // _CUBE_EFFECTS_H_
