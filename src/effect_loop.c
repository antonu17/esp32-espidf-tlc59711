#define LOG_LOCAL_LEVEL ESP_LOG_WARN

#include "effect_loop.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "effect_list.h"
#include "effects/effects.h"
#include "event_loop.h"

#define EFFECT_TIMEOUT pdMS_TO_TICKS(15000)

void effect_start(void *arg);
void effect_stop(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data);

TaskHandle_t effect_loop_task_handle;
effect_list_t *effects;

void effect_start(void *arg) {
    effect_t *effect = (effect_t *)arg;
    effect->function(effect);
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop));
    xTaskNotify(effect_loop_task_handle, 0, eNoAction);
    ESP_LOGD(effect->name, "notification sent (%p)", effect);
    vTaskDelay(portMAX_DELAY);
}

void effect_stop(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    effect_t *effect = (effect_t *)handler_arg;
    ESP_LOGD(effect->name, "stop event received (%p)", effect);
    effect->running = 0;
}

void effect_run(effect_t *effect) {
    TaskHandle_t handle;
    int effect_finished = 0;
    ESP_LOGI(effect->name, "start effect (%p)", effect);
    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop, effect));
    effect->running = 1;

    xTaskCreatePinnedToCore(effect_start, effect->name, 10000, effect, 1, &handle, 0);

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

    vTaskDelete(handle);
    ESP_LOGD(effect->name, "effect task deleted (%p)", effect);

    if (effect->stop_hook) {
        ESP_LOGD(effect->name, "stop hook: %s\t\t(%p)", effect->name, effect);
        effect->stop_hook(effect);
    }
}

void effect_loop() {
    effect_run(effect_new("test", test, effect_free));
    for (;;) {
        if (!effects->len) {
            vTaskDelay(10);
        }
        for (int i = 0; i < effects->len; i++) {
            effect_run(effects->effect[i]);
        }
    }
}

void init_effects() {
    effects = effect_list_new();
    effects = effect_list_add(effects, effect_list->effect[0]);
    xTaskCreatePinnedToCore(effect_loop, "effect_loop", 2048, NULL, 2, &effect_loop_task_handle, 0);
}
