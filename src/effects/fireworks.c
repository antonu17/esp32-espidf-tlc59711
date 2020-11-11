// #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <esp_event.h>
#include <esp_log.h>
#include <math.h>

#include "effects.h"

#define TAG __FILE__

#define SPEED pdMS_TO_TICKS(80)
#define DELAY pdMS_TO_TICKS(800)

static int running;

void fireworks_stop() {
    ESP_LOGI(TAG, "stop event received");
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, fireworks_stop));
    running = 0;
}

void fireworks() {
    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, fireworks_stop, NULL));
    running = 1;
    uint8_t n = 50;

    fb_clear();
    while (running) {
        fb_clear();

        uint8_t f, e;

        float origin_x = 3;
        float origin_y = 3;
        float origin_z = 3;

        uint8_t rand_y, rand_x, rand_z;

        float slowrate, gravity;

        // Particles and their position, x,y,z and their movement, dx, dy, dz
        float particles[n][6];

        origin_x = rand() % 4;
        origin_y = rand() % 4;
        origin_z = rand() % 2;
        origin_z += 5;
        origin_x += 2;
        origin_y += 2;

        // shoot a particle up in the air
        for (e = 0; e < origin_z; e++) {
            fb_set_pixel(origin_x, origin_y, e, WHITE);
            vTaskDelay(pdMS_TO_TICKS(60 + 50 * e));
            fb_clear();
        }

        // Fill particle array
        for (f = 0; f < n; f++) {
            // Position
            particles[f][0] = origin_x;
            particles[f][1] = origin_y;
            particles[f][2] = origin_z;

            rand_x = rand() % 200;
            rand_y = rand() % 200;
            rand_z = rand() % 200;

            // Movement
            particles[f][3] = 1 - (float)rand_x / 100;  // dx
            particles[f][4] = 1 - (float)rand_y / 100;  // dy
            particles[f][5] = 1 - (float)rand_z / 100;  // dz
            ESP_LOGD(TAG, "fill position and movement: %d", f);
        }

        // explode
        for (e = 0; e < 25; e++) {
            slowrate = 1 + tan((e + 0.1) / 20) * 10;
            gravity = tan((e + 0.1) / 20) / 2;

            ESP_LOGD(TAG, "calc gravity and slowness: %d", e);

            for (f = 0; f < n; f++) {
                particles[f][0] += particles[f][3] / slowrate;
                particles[f][1] += particles[f][4] / slowrate;
                particles[f][2] += particles[f][5] / slowrate;
                particles[f][2] -= gravity;

                fb_set_pixel(particles[f][0], particles[f][1], particles[f][2], WHITE);
                ESP_LOGD(TAG, "draw position and movement: %d", f);
            }

            vTaskDelay(SPEED);
            fb_clear();
        }
    }
    fb_clear();

    ESP_LOGD(TAG, "notify effect_loop");
    xTaskNotify(effect_loop_task_handle, 0, eNoAction);
    vTaskDelay(portMAX_DELAY);
}
