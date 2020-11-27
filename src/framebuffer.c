#include "framebuffer.h"

#include <driver/gpio.h>
#include <esp_timer.h>

#include "tlc59711.h"

rgb_t frame_buffer[512];
static uint8_t rows[] = {A0, A1, A2, A3, A4, A5, A6, A7};
static esp_timer_handle_t write_row_timer;

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

    ESP_ERROR_CHECK(esp_timer_create(&write_row_timer_args, &write_row_timer));
    start_framebuffer();
}

void start_framebuffer() {
    // Write to the TLC59711 stack via the High Resolution Timer every 1250us (~100 fps)
    ESP_ERROR_CHECK(esp_timer_start_periodic(write_row_timer, 1250));
}

void stop_framebuffer() {
    for (int i = 0; i < 8; i++) {
        gpio_set_level(rows[i], 0);
    }

    ESP_ERROR_CHECK(esp_timer_stop(write_row_timer));
}
