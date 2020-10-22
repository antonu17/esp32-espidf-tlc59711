#include <esp_event.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "effects.h"
#include "event_loop.h"

#define EFFECT_TIMEOUT pdMS_TO_TICKS(15000)

static const char *TAG = "EFFECTS";
esp_event_loop_handle_t effects_event_loop;
QueueHandle_t effects_queue = NULL;

TaskFunction_t effect_functions[] = {
    shift_suspend,
    vertical_travelers,
    cube_color_wheel,
    wave_color_wheel,
    rain,
    shift_planes,
    mic_spectrum,
};

void effect_loop() {
    TaskHandle_t xHandle = NULL;
    int effects = sizeof effect_functions / sizeof *effect_functions;
    uint8_t end;

    ESP_LOGI(TAG, "Number of effects: %d", effects);
    while (1) {
        for (int i = 0; i < effects; i++) {
            ESP_LOGI(TAG, "Start effect: %d", i);
            xTaskCreate(effect_functions[i], "current_effect", 4096, NULL, tskIDLE_PRIORITY, &xHandle);
            configASSERT(xHandle);
            if (!xQueueReceive(effects_queue, &end, EFFECT_TIMEOUT)) {
                ESP_LOGI(TAG, "Timing out effect: %d", i);
            }
            if (xHandle != NULL) {
                ESP_ERROR_CHECK(esp_event_post_to(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, NULL, 0, portMAX_DELAY));
                vTaskDelete(xHandle);
            }
        }
    }
}

void init_effects() {
    effects_queue = xQueueCreate(1, sizeof(uint8_t));
    xTaskCreate(effect_loop, "effect_loop", 4096, NULL, tskIDLE_PRIORITY, NULL);
}
