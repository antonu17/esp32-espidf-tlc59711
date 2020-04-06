#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "colors.h"
#include "tlc59711.h"

static void write_row(void *arg);
static void effect_loop();

static rgb_t frame_buffer[512];
static tlc_config_t config = {
    .num_drivers = 16,
    .data_pin = 23,
    .sclk_pin = 18,
    .spi_mode = 0,
    .spi_clock_speed_hz = 10000000,  // 10 MHz
};

void app_main() {
    const esp_timer_create_args_t write_row_timer_args = {
        .callback = &write_row,
        .name = "write_row",
    };
    esp_timer_handle_t write_row_timer;

    tlc_init(&config);
    ESP_ERROR_CHECK(esp_timer_create(&write_row_timer_args, &write_row_timer));

    // Effect loop function is running as a low priority task
    xTaskCreate(effect_loop, "effect_loop", 4096, NULL, tskIDLE_PRIORITY, NULL);

    // Write to the TLC59711 stack via the High Resolution Timer every 500us (~250 fps)
    ESP_ERROR_CHECK(esp_timer_start_periodic(write_row_timer, 500));
}

static void write_row(void *arg) {
    static uint8_t current_row = 0;
    static bool spi_running = false;

    if (spi_running)
        tlc_wait_write();
    else
        spi_running = true;

    /*
        The Real display function is currently commented out
        to test frame deflection emulation.
    */
    rgb_t *row = frame_buffer + current_row * 64;
    for (int i = 0; i < 64; i++) {
        rgb_t px = *row++;
        tlc_set_led(i, px.r, px.g, px.b);
    }

    /* Emulate frame deflection:
       - light up only one LED at a time.
       - display the first pixel from the current row
    */
    tlc_reset_leds();
    rgb_t px = frame_buffer[current_row * 64];
    tlc_set_led(current_row, px.r, px.g, px.b);

    tlc_queue_write();

    if (++current_row > 7)
        current_row = 0;
}

static void effect_loop() {
    rgb_t rgb;
    while (1) {
        for (uint8_t i = 0; i < 255; i++) {
            rgb_t rgb_hsv = hue_to_rgb_hsv(i);
            rgb_t rgb_linear = hue_to_rgb_linear(i);
            rgb_t rgb_sine = hue_to_rgb_sine(i);
            rgb_t rgb_sine2 = hue_to_rgb_sine2(i);

            for (int row = 0; row < 8; row++) {
                switch (row) {
                    case 0:
                    case 4:
                        rgb = rgb_hsv;
                        break;

                    case 1:
                    case 5:
                        rgb = rgb_linear;
                        break;

                    case 2:
                    case 6:
                        rgb = rgb_sine;
                        break;

                    case 3:
                    case 7:
                        rgb = rgb_sine2;
                        break;
                }

                for (int x = 0; x < 64; x++) {
                    frame_buffer[row * 64 + x] = rgb;
                }
            }

            vTaskDelay(20 / portTICK_PERIOD_MS);
        }
    }
}
