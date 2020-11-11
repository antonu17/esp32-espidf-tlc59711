#include "framebuffer.h"

#include <driver/gpio.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "tlc59711.h"

rgb_t frame_buffer[512];
rgb_t frame_buffer_draft[512];
static uint8_t rows[] = {A0, A1, A2, A3, A4, A5, A6, A7};

void IRAM_ATTR write_row(void *arg) {
    static uint8_t previous_row = 0, current_row = 0;

    rgb_t *row = frame_buffer + ((7 - current_row) * 64);
    for (int i = 0; i < 64; i++) {
        rgb_t px = *row++;
        tlc_set_led(i, px.r, px.g, px.b);
    }

    gpio_set_level(rows[previous_row], 0);
    tlc_oneshot_write();
    gpio_set_level(rows[current_row], 1);

    previous_row = current_row++;
    if (current_row > 7)
        current_row = 0;
}

void init_tlc() {
    tlc_config_t config = {
        .num_drivers = 16,
        .data_pin = 23,
        .sclk_pin = 18,
        .spi_mode = 0,
        .spi_clock_speed_hz = 20000000,  // 20 MHz
    };

    tlc_init(&config);
    tlc_set_brightness(BRIGHTNESS_RED, BRIGHTNESS_GREEN, BRIGHTNESS_BLUE);
}

void init_framebuffer() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    for (int i = 0; i < 8; i++) {
        gpio_set_level(rows[i], 0);
    }

    init_tlc();
    const esp_timer_create_args_t write_row_timer_args = {
        .callback = &write_row,
        .name = "write_row",
    };
    esp_timer_handle_t write_row_timer;

    ESP_ERROR_CHECK(esp_timer_create(&write_row_timer_args, &write_row_timer));

    // Write to the TLC59711 stack via the High Resolution Timer every 1250us (~100 fps)
    ESP_ERROR_CHECK(esp_timer_start_periodic(write_row_timer, 1250));
}

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
    bool should_read;
    for (z = 0; z < 8; z++) {
        for (y = 0; y < 8; y++) {
            for (x = 0; x < 8; x++) {
                c = (rgb_t){0, 0, 0};
                should_read = false;
                get_px = 0;
                set_px = 0;
                switch (direction) {
                    case FB_SHIFT_FORWARD:
                        get_px = 7 - x - 1;
                        set_px = 7 - x;
                        if (set_px > 0) should_read = true;
                        break;
                    case FB_SHIFT_BACK:
                        get_px = x + 1;
                        set_px = x;
                        if (set_px < 7) should_read = true;
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
