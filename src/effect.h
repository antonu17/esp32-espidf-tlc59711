#if !defined(EFFECT_H)
#define EFFECT_H

typedef struct effect effect_t;

typedef void (*effect_function_t)(void *);
typedef void (*stop_hook_t)(effect_t *effect);

typedef struct effect {
    const char *name;
    effect_function_t function;
    stop_hook_t stop_hook;
    int running;
} effect_t;

effect_t *effect_new(char *name, effect_function_t function, stop_hook_t stop_hook);
void effect_free(effect_t *effect);

#endif  // EFFECT_H
