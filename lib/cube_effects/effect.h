#if !defined(EFFECT_H)
#define EFFECT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct effect effect_t;

typedef void (*effect_function_t)(effect_t *effect);
typedef void (*stop_hook_t)(effect_t *effect);

typedef struct effect {
    const char *name;
    effect_function_t function;
    stop_hook_t stop_hook;
    volatile int running;
} effect_t;

effect_t *effect_new(const char *name, effect_function_t function, stop_hook_t stop_hook);
void effect_free(effect_t *effect);

#ifdef __cplusplus
}
#endif

#endif  // EFFECT_H
