#include "draw.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "colors.h"
#include "framebuffer.h"

static rgb_t frame_buffer_draft[512];

void argorder(uint8_t ix1, uint8_t ix2, uint8_t *ox1, uint8_t *ox2) {
    if (ix1 > ix2) {
        uint8_t tmp;
        tmp = ix1;
        ix1 = ix2;
        ix2 = tmp;
    }
    *ox1 = ix1;
    *ox2 = ix2;
}

void fb_clear() {
    memset(frame_buffer, 0, sizeof(frame_buffer));
}

void fb_clear_draft() {
    memset(frame_buffer_draft, 0, sizeof(frame_buffer));
}

rgb_t fb_get_pixel(uint8_t x, uint8_t y, uint8_t z) {
    int x_axis = x;
    int y_axis = y * 8;
    int z_axis = z * 64;

    return frame_buffer[x_axis + y_axis + z_axis];
}

int in_range(uint8_t x, uint8_t y, uint8_t z) {
    if (x < 8 && y < 8 && z < 8) {
        return 0x01;
    } else {
        return 0x00;
    }
}

void fb_set_pixel(uint8_t x, uint8_t y, uint8_t z, rgb_t c) {
    if (!in_range(x, y, z)) {
        return;
    }
    int x_axis = x;
    int y_axis = y * 8;
    int z_axis = z * 64;

    frame_buffer[x_axis + y_axis + z_axis] = c;
}

void fb_set_pixel_draft(uint8_t x, uint8_t y, uint8_t z, rgb_t c) {
    if (!in_range(x, y, z)) {
        return;
    }
    int x_axis = x;
    int y_axis = y * 8;
    int z_axis = z * 64;

    frame_buffer_draft[x_axis + y_axis + z_axis] = c;
}

void fb_shift(fb_axis_t axis, fb_shift_direction_t direction) {
    uint8_t get_px, set_px, x, y, z;
    rgb_t c;
    int should_read;
    for (z = 0; z < 8; z++) {
        for (y = 0; y < 8; y++) {
            for (x = 0; x < 8; x++) {
                c = (rgb_t){0, 0, 0};
                should_read = 0;
                get_px = 0;
                set_px = 0;
                switch (direction) {
                    case FB_SHIFT_FORWARD:
                        get_px = 7 - x - 1;
                        set_px = 7 - x;
                        if (set_px > 0) should_read = 1;
                        break;
                    case FB_SHIFT_BACK:
                        get_px = x + 1;
                        set_px = x;
                        if (set_px < 7) should_read = 1;
                        break;
                    default:
                        break;
                }
                switch (axis) {
                    case FB_AXIS_X:
                        if (should_read) c = fb_get_pixel(get_px, y, z);
                        fb_set_pixel(set_px, y, z, c);
                        break;
                    case FB_AXIS_Y:
                        if (should_read) c = fb_get_pixel(y, get_px, z);
                        fb_set_pixel(y, set_px, z, c);
                        break;
                    case FB_AXIS_Z:
                        if (should_read) c = fb_get_pixel(y, z, get_px);
                        fb_set_pixel(y, z, set_px, c);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void fb_set_plane(fb_axis_t axis, uint8_t x, rgb_t c) {
    int y, z;
    for (y = 0; y < 8; y++) {
        for (z = 0; z < 8; z++) {
            switch (axis) {
                case FB_AXIS_X:
                    fb_set_pixel(x, y, z, c);
                    break;
                case FB_AXIS_Y:
                    fb_set_pixel(y, x, z, c);
                    break;
                case FB_AXIS_Z:
                    fb_set_pixel(y, z, x, c);
                    break;
                default:
                    break;
            }
        }
    }
}

void fb_draw_wireframe(uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2, rgb_t c) {
    argorder(x1, x2, &x1, &x2);
    argorder(y1, y2, &y1, &y2);
    argorder(z1, z2, &z1, &z2);

    for (uint8_t px = x1; px <= x2; px++) {
        fb_set_pixel(px, y1, z1, c);
        fb_set_pixel(px, y1, z2, c);
        fb_set_pixel(px, y2, z1, c);
        fb_set_pixel(px, y2, z2, c);
    }
    for (uint8_t px = y1 + 1; px < y2; px++) {
        fb_set_pixel(x1, px, z1, c);
        fb_set_pixel(x1, px, z2, c);
        fb_set_pixel(x2, px, z1, c);
        fb_set_pixel(x2, px, z2, c);
    }
    for (uint8_t px = z1 + 1; px < z2; px++) {
        fb_set_pixel(x1, y1, px, c);
        fb_set_pixel(x1, y2, px, c);
        fb_set_pixel(x2, y1, px, c);
        fb_set_pixel(x2, y2, px, c);
    }
}

void fb_swap_pixels(uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2) {
    rgb_t tmp = fb_get_pixel(x2, y2, z2);
    fb_set_pixel(x2, y2, z2, fb_get_pixel(x1, y1, z1));
    fb_set_pixel(x1, y1, z1, tmp);
}

int fb_pixel_is_off(uint8_t x, uint8_t y, uint8_t z) {
    rgb_t c = fb_get_pixel(x, y, z);
    return c.r == 0 && c.g == 0 && c.b == 0;
}

void fb_draw_draft() {
    memcpy(frame_buffer, frame_buffer_draft, sizeof(frame_buffer));
}

float distance2d(float x1, float y1, float x2, float y2) {
    float dist;
    dist = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));

    return dist;
}

float distance3d(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dist;
    dist = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2));

    return dist;
}

void line_3d(int x1, int y1, int z1, int x2, int y2, int z2, rgb_t color) {
    int i, dx, dy, dz, l, m, n, x_inc, y_inc, z_inc,
        err_1, err_2, dx2, dy2, dz2;
    int pixel[3];
    pixel[0] = x1;
    pixel[1] = y1;
    pixel[2] = z1;
    dx = x2 - x1;
    dy = y2 - y1;
    dz = z2 - z1;
    x_inc = (dx < 0) ? -1 : 1;
    l = abs(dx);
    y_inc = (dy < 0) ? -1 : 1;
    m = abs(dy);
    z_inc = (dz < 0) ? -1 : 1;
    n = abs(dz);
    dx2 = l << 1;
    dy2 = m << 1;
    dz2 = n << 1;
    if ((l >= m) && (l >= n)) {
        err_1 = dy2 - l;
        err_2 = dz2 - l;
        for (i = 0; i < l; i++) {
            fb_set_pixel_draft(pixel[0], pixel[1], pixel[2], color);
            if (err_1 > 0) {
                pixel[1] += y_inc;
                err_1 -= dx2;
            }
            if (err_2 > 0) {
                pixel[2] += z_inc;
                err_2 -= dx2;
            }
            err_1 += dy2;
            err_2 += dz2;
            pixel[0] += x_inc;
        }
    } else if ((m >= l) && (m >= n)) {
        err_1 = dx2 - m;
        err_2 = dz2 - m;
        for (i = 0; i < m; i++) {
            fb_set_pixel_draft(pixel[0], pixel[1], pixel[2], color);
            if (err_1 > 0) {
                pixel[0] += x_inc;
                err_1 -= dy2;
            }
            if (err_2 > 0) {
                pixel[2] += z_inc;
                err_2 -= dy2;
            }
            err_1 += dx2;
            err_2 += dz2;
            pixel[1] += y_inc;
        }
    } else {
        err_1 = dy2 - n;
        err_2 = dx2 - n;
        for (i = 0; i < n; i++) {
            fb_set_pixel_draft(pixel[0], pixel[1], pixel[2], color);
            if (err_1 > 0) {
                pixel[1] += y_inc;
                err_1 -= dz2;
            }
            if (err_2 > 0) {
                pixel[0] += x_inc;
                err_2 -= dz2;
            }
            err_1 += dy2;
            err_2 += dx2;
            pixel[2] += z_inc;
        }
    }
    fb_set_pixel_draft(pixel[0], pixel[1], pixel[2], color);
}

void line_3d_float(point_t point1, point_t point2, rgb_t color) {
    float x1, y1, z1, x2, y2, z2;

    x1 = point1.x;
    y1 = point1.y;
    z1 = point1.z;
    x2 = point2.x;
    y2 = point2.y;
    z2 = point2.z;

    float i;
    float dx, dy, dz, l, m, n, x_inc, y_inc, z_inc,
        err_1, err_2, dx2, dy2, dz2;
    float pixel[3];
    pixel[0] = x1;
    pixel[1] = y1;
    pixel[2] = z1;
    dx = x2 - x1;
    dy = y2 - y1;
    dz = z2 - z1;
    x_inc = (dx < 0) ? -1 : 1;
    l = abs(dx);
    y_inc = (dy < 0) ? -1 : 1;
    m = abs(dy);
    z_inc = (dz < 0) ? -1 : 1;
    n = abs(dz);
    dx2 = l * l;
    dy2 = m * m;
    dz2 = n * n;
    if ((l >= m) && (l >= n)) {
        err_1 = dy2 - l;
        err_2 = dz2 - l;
        for (i = 0; i < l; i++) {
            fb_set_pixel_draft((int)pixel[0], (int)pixel[1], (int)pixel[2], color);
            if (err_1 > 0) {
                pixel[1] += y_inc;
                err_1 -= dx2;
            }
            if (err_2 > 0) {
                pixel[2] += z_inc;
                err_2 -= dx2;
            }
            err_1 += dy2;
            err_2 += dz2;
            pixel[0] += x_inc;
        }
    } else if ((m >= l) && (m >= n)) {
        err_1 = dx2 - m;
        err_2 = dz2 - m;
        for (i = 0; i < m; i++) {
            fb_set_pixel_draft((int)pixel[0], (int)pixel[1], (int)pixel[2], color);
            if (err_1 > 0) {
                pixel[0] += x_inc;
                err_1 -= dy2;
            }
            if (err_2 > 0) {
                pixel[2] += z_inc;
                err_2 -= dy2;
            }
            err_1 += dx2;
            err_2 += dz2;
            pixel[1] += y_inc;
        }
    } else {
        err_1 = dy2 - n;
        err_2 = dx2 - n;
        for (i = 0; i < n; i++) {
            fb_set_pixel_draft((int)pixel[0], (int)pixel[1], (int)pixel[2], color);
            if (err_1 > 0) {
                pixel[1] += y_inc;
                err_1 -= dz2;
            }
            if (err_2 > 0) {
                pixel[0] += x_inc;
                err_2 -= dz2;
            }
            err_1 += dy2;
            err_2 += dx2;
            pixel[2] += z_inc;
        }
    }
    fb_set_pixel_draft((int)pixel[0], (int)pixel[1], (int)pixel[2], color);
}
