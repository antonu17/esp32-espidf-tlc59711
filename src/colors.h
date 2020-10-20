#if !defined(COLORS_H)
#define COLORS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct rgb_t
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;

typedef struct hsv_t
{
    uint8_t h;
    uint8_t s;
    uint8_t v;
} hsv_t;

rgb_t hsv_to_rgb(hsv_t hsv);
rgb_t hue_to_rgb_hsv(uint8_t hue);
rgb_t hue_to_rgb_linear(uint8_t hue);
rgb_t hue_to_rgb_sine(uint8_t hue);
rgb_t hue_to_rgb_sine2(uint8_t hue);
int color_equals(rgb_t c1, rgb_t c2);

#endif  // COLORS_H
