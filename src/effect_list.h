#if !defined(EFFECT_LIST_H)
#define EFFECT_LIST_H

#include "effect.h"

typedef struct effect_list_t effect_list_t;

typedef struct effect_list_t {
    effect_t **effect;
    unsigned int len;
    unsigned int cap;
} effect_list_t;

effect_list_t *effect_list_new();
effect_list_t *effect_list_add(effect_list_t *effects, effect_t *effect);

extern effect_list_t *effect_list;
void init_effect_list();

#endif  // EFFECT_LIST_H
