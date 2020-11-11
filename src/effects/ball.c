#include <esp_event.h>
#include <esp_log.h>
#include <math.h>

#include "effects.h"

#define TAG __FILE__

#define SPEED pdMS_TO_TICKS(10)
#define DELAY pdMS_TO_TICKS(180)
#define STEP 20

void ball(effect_t *effect) {
    float origin_x, origin_y, origin_z, distance, diameter;
    uint8_t step = STEP;
    uint8_t c = 0;
    rgb_t color = hue_to_rgb_linear(c);

    origin_x = 0;
    origin_y = 3.5;
    origin_z = 3.5;

    diameter = 3;

    int x, y, z;
    uint16_t i = 0;

    fb_clear();
    while (effect->running) {
        i++;
        origin_x = 3.5 + sin((float)i / 50) * 2.5;
        origin_y = 3.5 + cos((float)i / 50) * 2.5;
        origin_z = 3.5 + cos((float)i / 30) * 2;

        diameter = 2 + sin((float)i / 150);

        if (!step--) {
            step = STEP;
            c++;
            color = hue_to_rgb_linear(c);
        }

        fb_clear_draft();
        for (x = 0; x < 8; x++) {
            for (y = 0; y < 8; y++) {
                for (z = 0; z < 8; z++) {
                    distance = distance3d(x, y, z, origin_x, origin_y, origin_z);
                    // printf("Distance: %f \n", distance);

                    if (distance > diameter && distance < diameter + 1) {
                        fb_set_pixel_draft(x, y, z, color);
                    }
                }
            }
        }
        fb_draw_draft();
        vTaskDelay(1);
    }
    fb_clear();
}
