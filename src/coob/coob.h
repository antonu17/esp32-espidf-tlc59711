#if !defined(COOB_H)
#define COOB_H

typedef struct coob* coob_t;

coob_t coob_get_instance(void);

void coob_mode_loop(coob_t instance);
void coob_mode_solo(coob_t instance);
void coob_switch_effect(coob_t instance, int effect);
int coob_get_mode(coob_t instance);

void init_coob();

#endif  // COOB_H
