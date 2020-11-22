#include "loop_state.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "coob_state.h"
#include "effect_functions.h"
#include "effect_launcher.h"
#include "effect_list.h"

/* Possible transition to the following state: */
#include "solo_state.h"

#define EFFECT_TIMEOUT pdMS_TO_TICKS(15000)

static void start_loop_mode(coob_state_t state);
static void stop_loop_mode();
static void effect_loop();

static int running = 0;

static void start_solo(coob_state_t state) {
    stop_loop_mode();

    transition_to_solo(state);
}

void transition_to_loop(coob_state_t state) {
    start_loop_mode(state);

    default_state(state);
    state->name = "loop";
    state->solo = start_solo;
    ESP_LOGD(__FILE__, "switched to loop mode");
}

static void start_loop_mode(coob_state_t state) {
    running = 1;
    xTaskCreatePinnedToCore(effect_loop, "effect_loop", 2048, NULL, 2, &effect_launcher_task_handle, 0);

}

static void stop_loop_mode() {
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

static void effect_loop() {
    // effect_run(effect_new("test", test, effect_free));
    for (;;) {
        if (0 == effect_list_length(effect_list)) {
            vTaskDelay(10);
            continue;
        }
        for (effect_t *e = effect_list_first(effect_list); !effect_list_done(effect_list, e); e = effect_list_next(effect_list, e)) {
            if (!running) {
                vTaskDelay(portMAX_DELAY);
            }
            effect_run(e, EFFECT_TIMEOUT);
        }
    }
}
