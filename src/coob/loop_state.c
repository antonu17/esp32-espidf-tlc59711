#include "loop_state.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "coob_state.h"
#include "effect_functions.h"
#include "effect_list.h"

/* Possible transition to the following state: */
#include "solo_state.h"

#define EFFECT_TIMEOUT pdMS_TO_TICKS(15000)

static void start_loop_mode();
static void stop_loop_mode();
static void effect_stop(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data);

static TaskHandle_t effect_loop_task_handle = NULL;
static TaskHandle_t current_effect_task_handle = NULL;
static int running = 0;

static void start_single(coob_state_t state) {
    stop_loop_mode();

    transition_to_solo(state);
}

void transition_to_loop(coob_state_t state) {
    start_loop_mode();

    default_state(state);
    state->single = start_single;
    ESP_LOGD(__FILE__, "switched to loop mode");
}

static void effect_start(void *arg) {
    effect_t *effect = (effect_t *)arg;
    effect->function(effect);
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop));
    xTaskNotify(effect_loop_task_handle, 0, eNoAction);
    ESP_LOGD(effect->name, "notification sent (%p)", effect);
    vTaskDelay(portMAX_DELAY);
}

static void effect_stop(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    effect_t *effect = (effect_t *)handler_arg;
    ESP_LOGD(effect->name, "stop event received (%p)", effect);
    effect->running = 0;
}

static void effect_run(effect_t *effect) {
    if (!running) {
        vTaskDelay(portMAX_DELAY);
    }

    int effect_finished = 0;
    ESP_LOGI(effect->name, "start effect (%p)", effect);
    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop, effect));
    effect->running = 1;

    xTaskCreatePinnedToCore(effect_start, effect->name, 10000, effect, 1, &current_effect_task_handle, 0);

    effect_finished = 0;
    if (xTaskNotifyWait(0, 0, NULL, EFFECT_TIMEOUT) == pdPASS) {
        ESP_LOGD(effect->name, "notification received (%p)", effect);
        effect_finished = 1;
    }

    if (!effect_finished) {
        ESP_LOGD(effect->name, "timeout effect (%p)", effect);
        ESP_ERROR_CHECK(esp_event_post_to(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, NULL, 0, portMAX_DELAY));
        ESP_LOGD(effect->name, "stop event posted (%p)", effect);
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
        ESP_LOGD(effect->name, "notification received (%p)", effect);
    }

    if (current_effect_task_handle) {
        vTaskDelete(current_effect_task_handle);
        ESP_LOGD(effect->name, "effect task deleted (%p)", effect);
    }

    if (effect->stop_hook) {
        ESP_LOGD(effect->name, "stop hook: %s\t\t(%p)", effect->name, effect);
        effect->stop_hook(effect);
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
            effect_run(e);
        }
    }
}

static void start_loop_mode() {
    running = 1;
    xTaskCreatePinnedToCore(effect_loop, "effect_loop", 2048, NULL, 2, &effect_loop_task_handle, 0);
}

static void stop_loop_mode() {
    running = 0;
    if (current_effect_task_handle) {
        vTaskDelete(current_effect_task_handle);
        ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop));
        ESP_LOGD(__FILE__, "effect task terminated");
    }
    if (effect_loop_task_handle) {
        vTaskDelete(effect_loop_task_handle);
        ESP_LOGD(__FILE__, "effect loop task terminated");
    }
}
