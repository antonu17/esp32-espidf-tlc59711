#if !defined(PERSISTENCE_H)
#define PERSISTENCE_H

#include "coob_state.h"

void save_mode(const coob_mode_t mode);
coob_mode_t load_mode();
void save_effect(const char* effect);
char* load_effect();

#endif  // PERSISTENCE_H
