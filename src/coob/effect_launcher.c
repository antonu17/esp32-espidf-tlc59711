#include "effect_launcher.h"
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <esp_log.h>
#include <freertos/semphr.h>

#include "event_loop.h"

void effect_stop_event_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data);

static TaskHandle_t current_effect_task_handle = NULL;
static SemaphoreHandle_t timeout_effect_task = NULL;

static void current_effect_task(void *arg) {
    effect_t *effect = (effect_t *)arg;
    effect->function(effect);
    xSemaphoreGive(timeout_effect_task);
    ESP_LOGD(__FILE__, "timeout semaphore given: %s (%p)", effect->name, effect);
    vTaskDelay(portMAX_DELAY);
}

void effect_stop_event_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    effect_t *effect = (effect_t *)handler_arg;
    ESP_LOGD(__FILE__, "stop event received: %s (%p)", effect->name, effect);
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop_event_handler));
    effect->running = 0;
}

void effect_run(effect_t *effect, TickType_t timeout) {
    int effect_finished;

    if (NULL == timeout_effect_task) {
        timeout_effect_task = xSemaphoreCreateBinary();
        if (NULL == timeout_effect_task) {
            ESP_LOGE(__FILE__, "timeout semaphore create failure");
            vTaskDelay(portMAX_DELAY);
        }
        ESP_LOGD(__FILE__, "timeout semaphore created");
    }

    ESP_LOGD(__FILE__, "start effect %s (%p)", effect->name, effect);
    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop_event_handler, effect));
    effect->running = 1;

    xTaskCreatePinnedToCore(current_effect_task, effect->name, 10000, effect, 1, &current_effect_task_handle, 0);
    effect_finished = 0;

    if (pdTRUE == xSemaphoreTake(timeout_effect_task, timeout)) {
        ESP_LOGD(__FILE__, "effect finished before timeout: %s (%p)", effect->name, effect);
        effect_finished = 1;
    }

    if (!effect_finished) {
        ESP_ERROR_CHECK(esp_event_post_to(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, NULL, 0, portMAX_DELAY));
        ESP_LOGD(__FILE__, "timeout effect, stop event posted: %s (%p)", effect->name, effect);
        xSemaphoreTake(timeout_effect_task, portMAX_DELAY);
        ESP_LOGD(__FILE__, "timeout semaphore taken: %s (%p)", effect->name, effect);
    }

    if (NULL != current_effect_task_handle) {
        effect_terminate(effect);
    }
}

void effect_terminate(effect_t *effect) {
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop_event_handler));

    if (NULL != current_effect_task_handle) {
        vTaskDelete(current_effect_task_handle);
        current_effect_task_handle = NULL;
        ESP_LOGD(__FILE__, "effect task terminated: %s (%p)", effect->name, effect);

        if (effect->stop_hook) {
            ESP_LOGD(__FILE__, "stop hook: %s (%p)", effect->name, effect);
            effect->stop_hook(effect);
        }
    }

    ESP_LOGD(__FILE__, "effect terminated: %s (%p)", effect->name, effect);
}
