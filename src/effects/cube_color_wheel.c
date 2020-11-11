#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "effects.h"

#define TAG __FILE__

#define SPEED pdMS_TO_TICKS(20)
#define DELAY pdMS_TO_TICKS(180)

static int running;
void cube_color_wheel_stop() {
    fb_clear();
    ESP_LOGI(TAG, "stop event received");
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, cube_color_wheel_stop));
    running = 0;
}

void cube_color_wheel() {
    rgb_t rgb;
    uint8_t c = 0;

    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, cube_color_wheel_stop, NULL));
    running = 1;

    fb_clear();
    while (running) {
        rgb = hue_to_rgb_sine2(c);
        for (int px = 0; px < 512; px++) {
            frame_buffer[px] = rgb;
        }
        c++;
        vTaskDelay(SPEED);
    }
    fb_clear();

    ESP_LOGI(TAG, "send notification effect_loop");
    xTaskNotify(effect_loop_task_handle, 0, eNoAction);
    vTaskDelay(portMAX_DELAY);
}
