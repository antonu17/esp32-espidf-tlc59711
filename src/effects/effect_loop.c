#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <esp_event.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "effects.h"
#include "event_loop.h"

#define EFFECT_TIMEOUT pdMS_TO_TICKS(15000)

#define TAG __FILE__

TaskHandle_t effect_loop_task_handle;

TaskFunction_t effect_functions[] = {
    sidewaves,
    shift_suspend,
    ripples,
    shift_planes,
    mic_spectrum,
    cube_color_wheel,
    vertical_travelers,
    wireframes,
    rain,
    wave_color_wheel,
};

void effect_loop() {
    int effect_finished = 0;
    TaskHandle_t xHandle = NULL;
    int effects = sizeof effect_functions / sizeof *effect_functions;

    ESP_LOGI(TAG, "Number of effects: %d", effects);

    for (;;) {
        for (int i = 0; i < effects; i++) {
            ESP_LOGI(TAG, "Start effect: %d", i);
            xTaskCreatePinnedToCore(effect_functions[i], "current_effect", 4096, NULL, 1, &xHandle, 0);

            effect_finished = 0;
            if (xTaskNotifyWait(0, 0, NULL, EFFECT_TIMEOUT) == pdPASS) {
                effect_finished = 1;
            }

            if (!effect_finished) {
                ESP_LOGI(TAG, "timeout effect: %d", i);
                ESP_ERROR_CHECK(esp_event_post_to(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, NULL, 0, portMAX_DELAY));
                ESP_LOGD(TAG, "stop event posted");
                xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
                ESP_LOGD(TAG, "notification received");
            }

            vTaskDelete(xHandle);
            ESP_LOGD(TAG, "effect task deleted");
        }
    }
}

void init_effects() {
    xTaskCreatePinnedToCore(effect_loop, "effect_loop", 4096, NULL, 2, &effect_loop_task_handle, 0);
}
