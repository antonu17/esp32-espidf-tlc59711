#if !defined(FRAMEBUFFER_H)
#define FRAMEBUFFER_H

#include <stdio.h>

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

#define BRIGHTNESS_RED 200
#define BRIGHTNESS_GREEN 200
#define BRIGHTNESS_BLUE 200

enum fb_shift_direction {
    FB_SHIFT_FORWARD = 0,
    FB_SHIFT_BACK
};
typedef enum fb_shift_direction fb_shift_direction_t;

enum fb_axis {
    FB_AXIS_X = 0,
    FB_AXIS_Y,
    FB_AXIS_Z
};
typedef enum fb_axis fb_axis_t;

void init_framebuffer();

/**
 * @brief Set a pixel on cube framebuffer
 *        x - (0 left <-> right 7)
 *        y - (0 top <-> bottom 7)
 *        z - (0 front <-> back 7)
 *
 *        Ex:
 *        left, top, front is 0, 0, 0
 *        right, top, front is 7, 0, 0
 *        right, bottom, front is 7, 7, 0
 *        right, bottom, back is 7, 7, 7
 */
void fb_set_pixel(uint8_t x, uint8_t y, uint8_t z, rgb_t c);
rgb_t fb_get_pixel(uint8_t x, uint8_t y, uint8_t z);
void fb_shift_x(fb_shift_direction_t direction);
void fb_shift(fb_axis_t axis, fb_shift_direction_t direction);
void fb_set_plane(fb_axis_t axis, uint8_t x, rgb_t c);
void fb_draw_wireframe(uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2, rgb_t c);
void fb_swap_pixels(uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2);
int fb_pixel_is_off(uint8_t x, uint8_t y, uint8_t z);
void fb_clear();

void fb_set_pixel_draft(uint8_t x, uint8_t y, uint8_t z, rgb_t c);
void fb_draw_draft();
void fb_clear_draft();

#endif  // FRAMEBUFFER_H
