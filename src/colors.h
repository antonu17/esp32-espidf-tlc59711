#if !defined(COLORS_H)
#define COLORS_H

#include <stdio.h>

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;

typedef struct
{
    uint8_t h;
    uint8_t s;
    uint8_t v;
} hsv_t;

rgb_t hsv_to_rgb(hsv_t hsv);

#endif  // COLORS_H
