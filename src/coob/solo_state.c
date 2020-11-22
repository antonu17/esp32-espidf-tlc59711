#include "solo_state.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "coob_state.h"

/* Possible transition to the following state: */
#include "effect_launcher.h"
#include "effect_list.h"
#include "loop_state.h"

static void start_solo_mode();
static void stop_solo_mode();
static void effect_solo();

static int running = 0;

static void start_loop(coob_state_t state) {
    stop_solo_mode();

    transition_to_loop(state);
}

void transition_to_solo(coob_state_t state) {
    start_solo_mode();

    default_state(state);
    state->name = "solo";
    state->loop = start_loop;
    ESP_LOGD(__FILE__, "switched to solo mode");
}

static void start_solo_mode() {
    running = 1;
    xTaskCreatePinnedToCore(effect_solo, "effect_solo", 2048, NULL, 2, &effect_launcher_task_handle, 0);
}

static void stop_solo_mode() {
    running = 0;
    if (current_effect_task_handle) {
        vTaskDelete(current_effect_task_handle);
        ESP_LOGD(__FILE__, "effect task terminated");
    }
    if (effect_launcher_task_handle) {
        vTaskDelete(effect_launcher_task_handle);
        ESP_LOGD(__FILE__, "effect loop task terminated");
    }
}

static void effect_solo() {
    // effect_run(effect_new("test", test, effect_free));
    for (;;) {
        if (0 == effect_list_length(effect_list)) {
            vTaskDelay(10);
            continue;
        }
        if (!running) {
            vTaskDelay(portMAX_DELAY);
        }
        effect_run(effect_list_first(effect_list), portMAX_DELAY);
    }
}
