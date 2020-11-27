#include "solo_state.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "coob_state.h"
#include "effect_launcher.h"
#include "effect_list.h"
#include "persistence.h"

/* Possible transition to the following state: */
#include "idle_state.h"
#include "loop_state.h"
#include "switching_state.h"

static void start_solo_mode();
static void stop_solo_mode();

static TaskHandle_t task = NULL;

static void start_loop(coob_state_t state) {
    stop_solo_mode(state);
    transition_to_loop(state);
}

static void start_idle(coob_state_t state) {
    stop_solo_mode(state);
    transition_to_idle(state);
}

static coob_err_t switch_effect(coob_state_t state, char *effect_name) {
    effect_t *e = NULL;
    e = effect_list_get_by_name(effect_list, effect_name);
    if (NULL == e) {
        ESP_LOGD(__FILE__, "effect not found: %s", effect_name);
        return COOB_ERR_EFFECT_NOT_FOUND;
    }
    stop_solo_mode(state);
    transition_to_switching(state, e, transition_to_solo);
    return COOB_ERR_OK;
}

void transition_to_solo(coob_state_t state) {
    xTaskCreatePinnedToCore(start_solo_mode, "start_solo_mode", 4096, state, 2, &task, 0);

    default_state(state);
    state->name = "solo";
    state->idle = start_idle;
    state->loop = start_loop;
    state->switch_effect = switch_effect;
    state->current_mode = SOLO;
    save_mode(SOLO);
    ESP_LOGD(__FILE__, "switched to solo mode");
}

static void start_solo_mode(coob_state_t state) {
    for (;;) {
        if (NULL == state->current_effect) {
            ESP_LOGE(__FILE__, "state->current_effect is NULL");
            vTaskDelay(portMAX_DELAY);
        }
        effect_run(state->current_effect, portMAX_DELAY);
    }
}

static void stop_solo_mode(coob_state_t state) {
    if (task) {
        ESP_LOGD(__FILE__, "terminating solo mode task: %p", task);
        vTaskDelete(task);
        task = NULL;
    }
    effect_terminate(state->current_effect);
    ESP_LOGD(__FILE__, "stopped solo mode");
}
