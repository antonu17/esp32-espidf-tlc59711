#if !defined(EFFECT_LIST_H)
#define EFFECT_LIST_H

#include "effect.h"

typedef struct effect_list *effect_list_t;

effect_list_t effect_list_new();
effect_list_t effect_list_add(effect_list_t list, effect_t *effect);

extern effect_list_t effect_list;

effect_t *effect_list_first(effect_list_t list);
int effect_list_done(effect_list_t list, effect_t *e);
effect_t *effect_list_next(effect_list_t list, effect_t *e);
int effect_list_length(effect_list_t list);
effect_t *effect_list_get_by_idx(effect_list_t list, int i);
effect_t *effect_list_get_by_name(effect_list_t list, char *effect_name);
void init_effect_list();

#endif  // EFFECT_LIST_H
