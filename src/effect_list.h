#if !defined(EFFECT_LIST_H)
#define EFFECT_LIST_H

typedef struct effect_t effect_t;
typedef struct effect_list_t effect_list_t;

typedef void (*effect_function_t)(void *);
typedef void (*stop_hook_t)(effect_t *effect);

typedef struct effect_t {
    const char *name;
    effect_function_t function;
    stop_hook_t stop_hook;
    int running;
} effect_t;

typedef struct effect_list_t {
    effect_t **effect;
    unsigned int len;
    unsigned int cap;
} effect_list_t;

effect_t *effect_new(char *name, effect_function_t function, stop_hook_t stop_hook);
void effect_free(effect_t *effect);

effect_list_t *effect_list_new();
effect_list_t *effect_list_add(effect_list_t *effects, effect_t *effect);

extern effect_list_t *effect_list;
void init_effect_list();

#endif  // EFFECT_LIST_H
