#if !defined(COOB_STATE_H)
#define COOB_STATE_H
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

typedef struct coob_state* coob_state_t;

typedef void (*effect_loop_func_t)(coob_state_t);
typedef void (*effect_single_func_t)(coob_state_t);

struct coob_state {
    effect_loop_func_t loop;
    effect_single_func_t single;
};

void default_state(coob_state_t state);

#endif  // COOB_STATE_H
