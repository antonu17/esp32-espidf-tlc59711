#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "effect_loop.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdlib.h>

#include "effects/effects.h"
#include "event_loop.h"

#define EFFECT_TIMEOUT pdMS_TO_TICKS(15000)

TaskHandle_t effect_loop_task_handle;
static effect_list_t *effects;

void effect_start(void *arg);
void effect_stop(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data);

effect_t *effect_new(char *name, TaskFunction_t function, stop_hook_t stop_hook) {
    effect_t *effect = malloc(sizeof(effect_t));
    if (effect) {
        effect->name = name;
        effect->function = function;
        effect->stop_hook = stop_hook;
        ESP_LOGD(__FILE__, "create effect: %s\t\t(%p)", name, effect);
    }
    return effect;
}

void effect_free(effect_t *effect) {
    ESP_LOGD(__FILE__, "delete effect: %s\t\t(%p)", effect->name, effect);
    free(effect);
}

effect_list_t *effect_list_new() {
    effect_list_t *list = malloc(sizeof(effect_list_t));
    list->len = 0;
    list->cap = 10;
    list->effect = malloc(10 * sizeof(effect_t));
    return list;
}

effect_list_t *effect_list_add(effect_list_t *effects, effect_t *effect) {
    if (!effects) {
        return effects;
    }
    unsigned int cap = effects->cap;
    unsigned int len = effects->len;
    if (len == cap) {
        cap *= 2;
        effect_t *src = (effect_t *)effects->effect;
        effect_t *dst = malloc(cap * sizeof(effect_t));
        for (unsigned int i = 0; i < len; i++) {
            dst[i] = src[i];
        }
        free(src);
        effects->effect = (effect_t **)dst;
        effects->cap = cap;
    }
    effects->effect[len] = effect;
    effects->len += 1;
    return effects;
}

void effect_start(void *arg) {
    effect_t *effect = (effect_t *)arg;
    effect->function(effect);
    xTaskNotify(effect_loop_task_handle, 0, eNoAction);
    ESP_LOGD(effect->name, "notification sent (%p)", effect);
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, effect_stop));
    vTaskDelay(portMAX_DELAY);
}

void effect_stop(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    effect_t *effect = (effect_t *)handler_arg;
    ESP_LOGI(effect->name, "stop event received (%p)", effect);
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
        ESP_LOGI(effect->name, "timeout effect (%p)", effect);
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
        for (int i = 0; i < effects->len; i++) {
            effect_run(effects->effect[i]);
        }
    }
}

void init_effects() {
    effects = effect_list_new();
    effects = effect_list_add(effects, effect_new("fireworks", fireworks, NULL));
    effects = effect_list_add(effects, effect_new("ball", ball, NULL));
    effects = effect_list_add(effects, effect_new("linespin", linespin, NULL));
    effects = effect_list_add(effects, effect_new("shift_planes", shift_planes, NULL));
    effects = effect_list_add(effects, effect_new("ripples", ripples, NULL));
    effects = effect_list_add(effects, effect_new("sinelines", sinelines, NULL));
    effects = effect_list_add(effects, effect_new("shift_suspend", shift_suspend, NULL));
    effects = effect_list_add(effects, effect_new("sidewaves", sidewaves, NULL));
    effects = effect_list_add(effects, effect_new("mic_spectrum", mic_spectrum, mic_spectrum_stop));
    effects = effect_list_add(effects, effect_new("cube_color_wheel", cube_color_wheel, NULL));
    effects = effect_list_add(effects, effect_new("vertical_travelers", vertical_travelers, NULL));
    effects = effect_list_add(effects, effect_new("wireframes", wireframes, NULL));
    effects = effect_list_add(effects, effect_new("rain", rain, NULL));
    effects = effect_list_add(effects, effect_new("wave_color_wheel", wave_color_wheel, NULL));

    ESP_LOGI(__FILE__, "Number of effects: %d", effects->len);
    xTaskCreatePinnedToCore(effect_loop, "effect_loop", 2048, NULL, 2, &effect_loop_task_handle, 0);
}
