#include "idle_state.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "coob_state.h"
#include "draw.h"
#include "effect_list.h"
#include "framebuffer.h"

/* Possible transition to the following state: */
#include "loop_state.h"
#include "solo_state.h"
#include "switching_state.h"

static void start_idle_mode();
static void stop_idle_mode();

static TaskHandle_t task = NULL;

static void start_loop(coob_state_t state) {
    stop_idle_mode();
    transition_to_loop(state);
}

static void start_solo(coob_state_t state) {
    stop_idle_mode();
    transition_to_solo(state);
}

static coob_err_t switch_effect(coob_state_t state, char *effect_name) {
    effect_t *e = NULL;
    e = effect_list_get_by_name(effect_list, effect_name);
    if (NULL == e) {
        ESP_LOGD(__FILE__, "effect not found: %s", effect_name);
        return COOB_ERR_EFFECT_NOT_FOUND;
    }
    transition_to_switching(state, e, NULL);
    return COOB_ERR_OK;
}

void transition_to_idle(coob_state_t state) {
    start_idle_mode();
    default_state(state);
    state->name = "idle";
    state->loop = start_loop;
    state->solo = start_solo;
    state->switch_effect = switch_effect;
    state->current_mode = IDLE;
    ESP_LOGD(__FILE__, "switched to idle mode");
}

static void start_idle_mode() {
    fb_clear();
    stop_framebuffer();
}

static void stop_idle_mode() {
    start_framebuffer();
}
