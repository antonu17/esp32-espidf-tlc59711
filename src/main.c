#include "log.h"
static const char *TAG = "main.c";

#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "driver/gpio.h"
#include "tlc59711.h"

void write_row(void *arg);
void effect_loop();

void init_wifi();
void init_http();

static rgb_t frame_buffer[512];
static tlc_config_t config = {
    .num_drivers = 16,
    .data_pin = 23,
    .sclk_pin = 18,
    .spi_mode = 0,
    .spi_clock_speed_hz = 20000000,  // 20 MHz
};

#define A0 13
#define A1 12
#define A2 14
#define A3 27
#define A4 26
#define A5 25
#define A6 33
#define A7 32

#define GPIO_OUTPUT_PIN_SEL ((1ULL << A0) | (1ULL << A1) | (1ULL << A2) | (1ULL << A3) | (1ULL << A4) | (1ULL << A5) | (1ULL << A6) | (1ULL << A7))

void app_main() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    const esp_timer_create_args_t write_row_timer_args = {
        .callback = &write_row,
        .name = "write_row",
    };
    esp_timer_handle_t write_row_timer;

    tlc_init(&config);
    tlc_set_brightness(100,100,100);
    ESP_ERROR_CHECK(esp_timer_create(&write_row_timer_args, &write_row_timer));

    // Effect loop function is running as a low priority task
    xTaskCreatePinnedToCore(effect_loop, "effect_loop", 4096, NULL, tskIDLE_PRIORITY, NULL, PRO_CPU_NUM);

    // Write to the TLC59711 stack via the High Resolution Timer every 1250us (~100 fps)
    ESP_ERROR_CHECK(esp_timer_start_periodic(write_row_timer, 1250));

    init_wifi();
    init_http();
}

void write_row(void *arg) {
    static uint8_t previous_row = 0, current_row = 0;
    static uint8_t rows[] = {A0, A1, A2, A3, A4, A5, A6, A7};

    rgb_t *row = frame_buffer + current_row * 64;
    for (int i = 0; i < 64; i++) {
        rgb_t px = *row++;
        tlc_set_led(i, px.r, px.g, px.b);
    }

    // tlc_queue_write();
    // tlc_wait_write();
    gpio_set_level(rows[previous_row], 0);
    tlc_oneshot_write();
    gpio_set_level(rows[current_row], 1);

    previous_row = current_row++;
    if (current_row > 7)
        current_row = 0;
}

void effect_loop() {
    color_wheel();
}

void one_pixel() {
    int x = 63;
    int color = 0;
    while (1) {
        for (int row = 0; row < 8; row++) {
            frame_buffer[row * 64 + x] = hue_to_rgb_sine2(color);
            vTaskDelay(200 / portTICK_PERIOD_MS);
            frame_buffer[row * 64 + x] = (rgb_t){0, 0, 0};

            if (--x < 56)
                x = 63;

            if (++color > 255)
                color = 0;
        }
    }
}

void color_wheel() {
    rgb_t rgb;
    int c0 = 0, c1 = 10, c2 = 20, c3 = 30, c4 = 40, c5 = 50, c6 = 60, c7 = 70;
    while (1) {
        for (int row = 0; row < 8; row++) {
            switch (row) {
                case 0:
                    rgb = hue_to_rgb_sine2(c0);
                    break;
                case 1:
                    rgb = hue_to_rgb_sine2(c1);
                    break;
                case 2:
                    rgb = hue_to_rgb_sine2(c2);
                    break;
                case 3:
                    rgb = hue_to_rgb_sine2(c3);
                    break;
                case 4:
                    rgb = hue_to_rgb_sine2(c4);
                    break;
                case 5:
                    rgb = hue_to_rgb_sine2(c5);
                    break;
                case 6:
                    rgb = hue_to_rgb_sine2(c6);
                    break;
                case 7:
                    rgb = hue_to_rgb_sine2(c7);
                    break;
            }

            for (int x = 0; x < 64; x++) {
                frame_buffer[row * 64 + x] = rgb;
            }
        }

        if (++c0 > 255)
            c0 = 0;
        if (++c1 > 255)
            c1 = 0;
        if (++c2 > 255)
            c2 = 0;
        if (++c3 > 255)
            c3 = 0;
        if (++c4 > 255)
            c4 = 0;
        if (++c5 > 255)
            c5 = 0;
        if (++c6 > 255)
            c6 = 0;
        if (++c7 > 255)
            c7 = 0;

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void one_row() {
    rgb_t rgb;
    while (1) {
        for (int row = 0; row < 8; row++) {
            switch (row) {
                case 0:
                    rgb = (rgb_t){0, 0, 0};
                    break;
                case 1:
                    rgb = (rgb_t){0, 0, 0};
                    break;
                case 2:
                    rgb = (rgb_t){0, 0, 0};
                    break;
                case 3:
                    rgb = (rgb_t){0, 0, 0};
                    break;
                case 4:
                    rgb = (rgb_t){0, 0, 0};
                    break;
                case 5:
                    rgb = (rgb_t){255, 0, 0};
                    break;
                case 6:
                    rgb = (rgb_t){0, 0, 0};
                    break;
                case 7:
                    rgb = (rgb_t){0, 0, 0};
                    break;
            }

            for (int x = 0; x < 64; x++) {
                frame_buffer[row * 64 + x] = rgb;
            }
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
}
