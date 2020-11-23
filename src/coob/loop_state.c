#include "loop_state.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "coob_state.h"
#include "effect_functions.h"
#include "effect_launcher.h"
#include "effect_list.h"
#include "persistence.h"

/* Possible transition to the following state: */
#include "solo_state.h"
#include "switching_state.h"

#define EFFECT_TIMEOUT pdMS_TO_TICKS(15000)

static void start_loop_mode(coob_state_t state);
static void stop_loop_mode();

static TaskHandle_t task = NULL;

static void start_solo(coob_state_t state) {
    stop_loop_mode(state);

    transition_to_solo(state);
}

static void switch_effect(coob_state_t state) {
    effect_t *next_effect = effect_list_next(effect_list, state->current_effect);
    if (NULL == next_effect) {
        next_effect = effect_list_first(effect_list);
    }
    ESP_LOGD(__FILE__, "next effect is: %s", next_effect->name);

    stop_loop_mode(state);
    transition_to_switching(state, next_effect, transition_to_loop);
    vTaskDelete(NULL);
}

void transition_to_loop(coob_state_t state) {
    xTaskCreatePinnedToCore(start_loop_mode, "start_loop_mode", 4096, state, 2, &task, 0);
    ESP_LOGD(__FILE__, "start_loop_mode task started: %p", task);

    default_state(state);
    state->name = "loop";
    state->solo = start_solo;
    save_mode(LOOP);
    ESP_LOGD(__FILE__, "switched to loop mode");
}

static void start_loop_mode(coob_state_t state) {
    if (0 == effect_list_length(effect_list)) {
        ESP_LOGE(__FILE__, "effect_list is empty");
        vTaskDelay(portMAX_DELAY);
    }
    if (NULL == state->current_effect) {
        ESP_LOGE(__FILE__, "state->current_effect is NULL");
        vTaskDelay(portMAX_DELAY);
    }

    effect_run(state->current_effect, EFFECT_TIMEOUT);
    xTaskCreatePinnedToCore(switch_effect, "switch_effect", 4096, state, 2, NULL, 0);
    vTaskDelay(portMAX_DELAY);
}

static void stop_loop_mode(coob_state_t state) {
    if (task) {
        ESP_LOGD(__FILE__, "terminating loop mode task: %p", task);
        vTaskDelete(task);
        task = NULL;
    }
    effect_terminate(state->current_effect);
    ESP_LOGD(__FILE__, "stopped loop mode");
}
