#include <esp_event.h>
#include <esp_log.h>

#include "effects.h"

#define TAG __FILE__
#define DELAY pdMS_TO_TICKS(180)
#define LONG pdMS_TO_TICKS(10000)

static int running;

void test_stop() {
    ESP_LOGI(TAG, "stop event received");
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, test_stop));
    running = 0;
}

void test() {

    // ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, test_stop, NULL));

    running = 1;

    fb_clear();
    while (running) {
        // for (int i = 0; i < 512; i++) {
        //     frame_buffer[i] = WHITE;
        // }
        // vTaskDelay(LONG);

        for (int i = 0; i < 512; i++) {
            frame_buffer[i] = RED;
        }
        vTaskDelay(LONG);

        // for (int i = 0; i < 512; i++) {
        //     frame_buffer[i] = GREEN;
        // }
        // vTaskDelay(LONG);

        // for (int i = 0; i < 512; i++) {
        //     frame_buffer[i] = BLUE;
        // }
        // vTaskDelay(LONG);

        // for (int i = 0; i < 8; i++) {
        //     fb_clear();
        //     fb_set_plane(FB_AXIS_Z, i, WHITE);
        //     vTaskDelay(DELAY);
        // }
        // for (int i = 7; i >= 0; i--) {
        //     fb_clear();
        //     fb_set_plane(FB_AXIS_Z, i, RED);
        //     vTaskDelay(DELAY);
        // }
        // for (int i = 0; i < 8; i++) {
        //     fb_clear();
        //     fb_set_plane(FB_AXIS_Z, i, GREEN);
        //     vTaskDelay(DELAY);
        // }
        // for (int i = 7; i >= 0; i--) {
        //     fb_clear();
        //     fb_set_plane(FB_AXIS_Z, i, BLUE);
        //     vTaskDelay(DELAY);
        // }
    }
    fb_clear();

    ESP_LOGD(TAG, "notify effect_loop");
    xTaskNotify(effect_loop_task_handle, 0, eNoAction);
    vTaskDelay(portMAX_DELAY);
}
