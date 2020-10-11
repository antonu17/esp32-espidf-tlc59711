#if !defined(FRAMEBUFFER_H)
#define FRAMEBUFFER_H

#include "colors.h"

extern rgb_t frame_buffer[512];

#define A0 13
#define A1 12
#define A2 14
#define A3 27
#define A4 26
#define A5 25
#define A6 33
#define A7 32

#define GPIO_OUTPUT_PIN_SEL ((1ULL << A0) | (1ULL << A1) | (1ULL << A2) | (1ULL << A3) | (1ULL << A4) | (1ULL << A5) | (1ULL << A6) | (1ULL << A7))

#define BRIGHTNESS_RED 255
#define BRIGHTNESS_GREEN 255
#define BRIGHTNESS_BLUE 255

void init_framebuffer();

#endif  // FRAMEBUFFER_H
