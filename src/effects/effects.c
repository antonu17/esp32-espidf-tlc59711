#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp_event.h"
#include "esp_log.h"

#include "event_loop.h"
#include "effects.h"

static const char *TAG = "EFFECTS";
esp_event_loop_handle_t effects_event_loop;

TaskFunction_t effect_functions[] = {
    cube_color_wheel,
    wave_color_wheel,
    mic_spectrum};

void effect_loop() {
    TaskHandle_t xHandle = NULL;
    int effects = sizeof effect_functions / sizeof *effect_functions;

    ESP_LOGI(TAG, "Number of effects: %d", effects);
    while (1) {
        for (int i = 0; i < effects; i++) {
            ESP_LOGI(TAG, "Start effect: %d", i);
            xTaskCreate(effect_functions[i], "current_effect", 4096, NULL, tskIDLE_PRIORITY, &xHandle);
            configASSERT(xHandle);
            vTaskDelay(10000 / portTICK_PERIOD_MS);
            if (xHandle != NULL) {
                ESP_ERROR_CHECK(esp_event_post_to(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, NULL, 0, portMAX_DELAY));
                vTaskDelete(xHandle);
            }
        }
    }
}

void init_effects() {
    xTaskCreate(effect_loop, "effect_loop", 4096, NULL, tskIDLE_PRIORITY, NULL);
}
