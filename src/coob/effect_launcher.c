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
    ESP_LOGD(effect->name, "timeout semaphore given (%p)", effect);
    vTaskDelay(portMAX_DELAY);
}

void effect_stop_event_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    effect_t *effect = (effect_t *)handler_arg;
    ESP_LOGD(effect->name, "stop event received (%p)", effect);
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

    ESP_LOGI(effect->name, "start effect (%p)", effect);
    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop_event_handler, effect));
    effect->running = 1;

    xTaskCreatePinnedToCore(current_effect_task, effect->name, 10000, effect, 1, &current_effect_task_handle, 0);
    effect_finished = 0;

    if (pdTRUE == xSemaphoreTake(timeout_effect_task, timeout)) {
        ESP_LOGD(effect->name, "effect finished before timeout (%p)", effect);
        ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop_event_handler));
        effect_finished = 1;
    }

    if (!effect_finished) {
        ESP_ERROR_CHECK(esp_event_post_to(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, NULL, 0, portMAX_DELAY));
        ESP_LOGD(effect->name, "timeout effect, stop event posted (%p)", effect);
        xSemaphoreTake(timeout_effect_task, portMAX_DELAY);
        ESP_LOGD(effect->name, "timeout_effect_task taken (%p)", effect);
    }

    if (current_effect_task_handle) {
        vTaskDelete(current_effect_task_handle);
        current_effect_task_handle = NULL;
        ESP_LOGD(effect->name, "effect task deleted (%p)", effect);
    }

    if (effect->stop_hook) {
        ESP_LOGD(effect->name, "stop hook: %s\t\t(%p)", effect->name, effect);
        effect->stop_hook(effect);
    }
}

void effect_terminate(effect_t *effect) {
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop_event_handler));
    if (NULL != current_effect_task_handle) {
        vTaskDelete(current_effect_task_handle);
        current_effect_task_handle = NULL;

        if (effect->stop_hook) {
            ESP_LOGD(effect->name, "stop hook: %s\t\t(%p)", effect->name, effect);
            effect->stop_hook(effect);
        }
        ESP_LOGD(__FILE__, "effect task terminated");
    }
    ESP_LOGD(__FILE__, "effect terminated");
}
