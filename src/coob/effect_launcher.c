#include "effect_launcher.h"

#include <esp_log.h>

#include "event_loop.h"

void effect_stop(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data);

TaskHandle_t effect_launcher_task_handle = NULL;
TaskHandle_t current_effect_task_handle = NULL;

void effect_start(void *arg) {
    effect_t *effect = (effect_t *)arg;
    effect->function(effect);
    xTaskNotify(effect_launcher_task_handle, 0, eNoAction);
    ESP_LOGD(effect->name, "notification sent (%p)", effect);
    vTaskDelay(portMAX_DELAY);
}

void effect_stop(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    effect_t *effect = (effect_t *)handler_arg;
    ESP_LOGD(effect->name, "stop event received (%p)", effect);
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop));
    effect->running = 0;
}

void effect_run(effect_t *effect, TickType_t timeout) {
    int effect_finished = 0;
    ESP_LOGI(effect->name, "start effect (%p)", effect);
    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop, effect));
    effect->running = 1;

    xTaskCreatePinnedToCore(effect_start, effect->name, 10000, effect, 1, &current_effect_task_handle, 0);

    effect_finished = 0;
    if (xTaskNotifyWait(0, 0, NULL, timeout) == pdPASS) {
        ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop));
        ESP_LOGD(effect->name, "notification received (%p)", effect);
        effect_finished = 1;
    }

    if (!effect_finished) {
        ESP_ERROR_CHECK(esp_event_post_to(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, NULL, 0, portMAX_DELAY));
        ESP_LOGD(effect->name, "timeout effect (%p)", effect);
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
