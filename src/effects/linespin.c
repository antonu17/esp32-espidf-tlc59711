#include <esp_event.h>
#include <esp_log.h>
#include <math.h>

#include "effects.h"

#define STEP 10

void linespin(effect_t *effect) {
    fb_clear();
    float top_x, top_y, top_z, bot_x, bot_y, bot_z, sin_base;
    float center_x, center_y;
    rgb_t color;
    uint8_t c = 0, step = STEP;
    point_t p1, p2;

    center_x = 3.5;
    center_y = 3.5;

    int i, z;
    for (i = 0; i < 2000; i++) {
        //printf("Sin base %f \n",sin_base);

        if (!step--) {
            step = STEP;
            c++;
            color = hue_to_rgb_linear(c);
        }

        fb_clear_draft();
        for (z = 0; z < 8; z++) {
            sin_base = (float)i / 50 + (float)z / (10 + (7 * sin((float)i / 200)));

            top_x = center_x + sin(sin_base) * 5;
            top_y = center_x + cos(sin_base) * 5;

            bot_x = center_x + sin(sin_base + 3.14) * 10;
            bot_y = center_x + cos(sin_base + 3.14) * 10;

            bot_z = z;
            top_z = z;

            p1.x = top_x;
            p1.y = top_y;
            p1.z = top_z;
            p2.x = top_x;
            p2.y = top_y;
            p2.z = top_z;

            line_3d((int)top_z, (int)top_x, (int)top_y, (int)bot_z, (int)bot_x, (int)bot_y, color);
        }
        fb_draw_draft();

        vTaskDelay(5);
    }
    fb_clear();
}
