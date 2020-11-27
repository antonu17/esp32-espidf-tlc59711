#if !defined(COOB_H)
#define COOB_H

enum {
    COOB_ERR_OK = 0,
    COOB_ERR_EFFECT_NOT_FOUND,
};

typedef int coob_err_t;
typedef struct coob* coob_t;

coob_t coob_get_instance(void);

void coob_mode_idle(coob_t instance);
void coob_mode_loop(coob_t instance);
void coob_mode_solo(coob_t instance);
coob_err_t coob_switch_effect(coob_t instance, char* effect_name);
int coob_get_mode(coob_t instance);
char* coob_get_effect(coob_t instance);
char* coob_error_str(coob_err_t);

void init_coob();

#endif  // COOB_H
