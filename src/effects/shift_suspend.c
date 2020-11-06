#include <esp_event.h>
#include <esp_log.h>

#include "effects.h"

#define TAG __FILE__
#define DELAY pdMS_TO_TICKS(300)
#define SPEED pdMS_TO_TICKS(40)

static int running;

void shift_suspend_stop() {
    ESP_LOGI(TAG, "stop event received");
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, shift_suspend_stop));
    running = 0;
}

void draw_positions_axis(fb_axis_t axis, uint8_t positions[64], fb_shift_direction_t direction) {
    int x, y, p;
    rgb_t on = {255, 255, 255};

    fb_clear();

    for (x = 0; x < 8; x++) {
        for (y = 0; y < 8; y++) {
            if (direction == FB_SHIFT_FORWARD) {
                p = (7 - positions[(x * 8) + y]);
            } else {
                p = positions[(x * 8) + y];
            }

            if (axis == FB_AXIS_Z)
                fb_set_pixel(x, y, p, on);

            if (axis == FB_AXIS_Y)
                fb_set_pixel(x, p, y, on);

            if (axis == FB_AXIS_X)
                fb_set_pixel(p, y, x, on);
        }
    }
}

void _shift_suspend(fb_axis_t axis, fb_shift_direction_t direction) {
    uint8_t positions[64];
    uint8_t destinations[64];

    int i, px;

    // Set 64 random positions
    for (i = 0; i < 64; i++) {
        positions[i] = 0;  // Set all starting positions to 0
        destinations[i] = rand() % 8;
    }

    // Loop 8 times to allow destination 7 to reach all the way
    for (i = 0; i < 8; i++) {
        // For every iteration, move all position one step closer to their destination
        for (px = 0; px < 64; px++) {
            if (positions[px] < destinations[px]) {
                positions[px]++;
            }
        }
        // Draw the positions and take a nap
        draw_positions_axis(axis, positions, direction);
        vTaskDelay(SPEED);
    }

    // Set all destinations to 7 (opposite from the side they started out)
    for (i = 0; i < 64; i++) {
        destinations[i] = 7;
    }

    // Suspend the positions in mid-air for a while
    vTaskDelay(DELAY);

    // Then do the same thing one more time
    for (i = 0; i < 8; i++) {
        for (px = 0; px < 64; px++) {
            if (positions[px] < destinations[px]) {
                positions[px]++;
            }
            if (positions[px] > destinations[px]) {
                positions[px]--;
            }
        }
        draw_positions_axis(axis, positions, direction);
        vTaskDelay(SPEED);
    }
    vTaskDelay(DELAY);
}

void shift_suspend() {
    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, shift_suspend_stop, NULL));
    running = 1;

    fb_clear();
    _shift_suspend(FB_AXIS_X, FB_SHIFT_BACK);
    _shift_suspend(FB_AXIS_X, FB_SHIFT_FORWARD);
    _shift_suspend(FB_AXIS_Y, FB_SHIFT_BACK);
    _shift_suspend(FB_AXIS_Y, FB_SHIFT_FORWARD);
    _shift_suspend(FB_AXIS_Z, FB_SHIFT_BACK);
    _shift_suspend(FB_AXIS_Z, FB_SHIFT_FORWARD);
    fb_clear();

    ESP_LOGD(TAG, "notify effect_loop");
    xTaskNotify(effect_loop_task_handle, 0, eNoAction);
    vTaskDelay(portMAX_DELAY);
}
