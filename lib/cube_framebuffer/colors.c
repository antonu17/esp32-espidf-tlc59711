#include "colors.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// In this library HUE size is uint8_t - 256 steps.
// Because 256 divided by 3 is 85 and reminder 1,
// effectively it gives 255 only steps.
#define REGION_SIZE 85.0

#define HALF_PI 1.57079632
#define PI 3.14159265

rgb_t WHITE = {255, 255, 255};
rgb_t RED = {255, 0, 0};
rgb_t GREEN = {0, 255, 0};
rgb_t BLUE = {0, 0, 255};

rgb_t hsv_to_rgb(hsv_t hsv) {
    rgb_t rgb;
    float h, s, v, c, x, m;
    uint8_t C, X;

    h = hsv.h / 255.0;
    s = hsv.s / 255.0;
    v = hsv.v / 255.0;

    if (s == 0) {
        rgb.r = v * 255;
        rgb.g = v * 255;
        rgb.b = v * 255;
        return rgb;
    }

    h = fmod(h, 1.0);
    h = h * 6;
    c = v * s;
    x = c * (1 - fabs((fmod(h, 2)) - 1));
    m = v - c;

    C = (c + m) * 255;
    X = (x + m) * 255;

    if (h <= 1) {
        rgb.r = C;
        rgb.g = X;
        rgb.b = 0;
    } else if (h <= 2) {
        rgb.r = X;
        rgb.g = C;
        rgb.b = 0;
    } else if (h <= 3) {
        rgb.r = 0;
        rgb.g = C;
        rgb.b = X;
    } else if (h <= 4) {
        rgb.r = 0;
        rgb.g = X;
        rgb.b = C;
    } else if (h <= 5) {
        rgb.r = X;
        rgb.g = 0;
        rgb.b = C;
    } else {
        rgb.r = C;
        rgb.g = 0;
        rgb.b = X;
    }

    return rgb;
}

rgb_t hue_to_rgb_hsv(uint8_t hue) {
    return hsv_to_rgb((hsv_t){hue, 255, 255});
}

rgb_t hue_to_rgb_linear(uint8_t hue) {
    rgb_t rgb;

    uint8_t region, step, x;

    if (hue == 255)
        hue = 0;
    region = hue / REGION_SIZE;
    step = hue - REGION_SIZE * region;

    x = step * 3;

    switch (region) {
        case 0:
            rgb.r = 255 - x;
            rgb.g = x;
            rgb.b = 0;
            break;
        case 1:
            rgb.r = 0;
            rgb.g = 255 - x;
            rgb.b = x;
            break;
        default:
            rgb.r = x;
            rgb.g = 0;
            rgb.b = 255 - x;
    }

    return rgb;
}

rgb_t hue_to_rgb_sine(uint8_t hue) {
    rgb_t rgb;
    float h, r, g, b;

    h = hue / 255.0;

    h += 0.5;
    h *= -1;

    r = sin(PI * h);
    g = sin(PI * (h + 1 / 3.0));
    b = sin(PI * (h + 2 / 3.0));

    rgb.r = pow(r, 2) * 255;
    rgb.g = pow(g, 2) * 255;
    rgb.b = pow(b, 2) * 255;

    return rgb;
}

rgb_t hue_to_rgb_sine2(uint8_t hue) {
    rgb_t rgb;
    uint8_t region, step, x;
    float h;

    if (hue == 255)
        hue = 0;
    region = hue / REGION_SIZE;
    step = hue - REGION_SIZE * region;

    h = step / REGION_SIZE;
    x = pow(sin(HALF_PI * h), 2) * 255;

    switch (region) {
        case 0:
            rgb.r = 255 - x;
            rgb.g = x;
            rgb.b = 0;
            break;
        case 1:
            rgb.r = 0;
            rgb.g = 255 - x;
            rgb.b = x;
            break;
        default:
            rgb.r = x;
            rgb.g = 0;
            rgb.b = 255 - x;
    }

    return rgb;
}

int color_equals(rgb_t c1, rgb_t c2) {
    return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b;
}
