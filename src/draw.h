#if !defined(DRAW_H)
#define DRAW_H

#include "colors.h"
#include "framebuffer.h"

typedef struct {
	float x;
	float y;
	float z;
} point_t;

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

/**
 * @brief Set a pixel on cube framebuffer
 *        x - (0 left <-> right 7)
 *        y - (0 front <-> back 7)
 *        z - (0 bottom <-> top 7)
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

float distance2d(float x1, float y1, float x2, float y2);
float distance3d(float x1, float y1, float z1, float x2, float y2, float z2);
void line_3d(int x1, int y1, int z1, int x2, int y2, int z2, rgb_t color);
void line_3d_float (point_t point1, point_t point2, rgb_t color);

#endif  // DRAW_H
