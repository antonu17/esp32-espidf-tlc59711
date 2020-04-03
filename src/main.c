#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>

#include "tlc59711.h"
#include "colors.h"

static void write_row(void *arg);
static void effect_loop();

static rgb_t frame_buffer[512];
static tlc_config_t config = {
    .num_drivers = 16,
    .data_pin = 23,
    .sclk_pin = 18,
    .spi_mode = 0,
    .spi_clock_speed_hz = 10000000, // 10 MHz
};

void app_main()
{
    const esp_timer_create_args_t write_row_timer_args = {
        .callback = &write_row,
        .name = "write_row",
    };
    esp_timer_handle_t write_row_timer;

    tlc_init(&config);
    ESP_ERROR_CHECK(esp_timer_create(&write_row_timer_args, &write_row_timer));

    // Effect loop function is running as a low priority task
    xTaskCreate(effect_loop, "effect_loop", 4096, NULL, tskIDLE_PRIORITY, NULL);

    // Write to the TLC59711 stack via the High Resolution Timer every 2000us (~60 fps)
    ESP_ERROR_CHECK(esp_timer_start_periodic(write_row_timer, 2000));
}

static void write_row(void *arg)
{
    static uint8_t current_row = 0;
    static bool spi_running = false;

    rgb_t *row = frame_buffer + current_row * 64;

    /* Emulate frame deflection:
       - light up only one LED at a time.
       - display the first pixel from the current row
    */
    uint8_t leds[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    rgb_t px = *row;
    tlc_reset_leds();
    tlc_set_led(leds[current_row], px.r, px.g, px.b);

    /*
        The Real display function is currently commented out
        to test frame deflection emulation.

        for (int i = 0; i < 64; i++)
        {
            rgb_t px = *row++;
            tlc_set_led(i, px.r, px.g, px.b);
        }
    */

    if (spi_running)
        tlc_wait_write();
    else
        spi_running = true;

    tlc_queue_write();

    if (++current_row > 7)
        current_row = 0;
}

static void effect_loop()
{
    while (1)
    {
        for (uint8_t i = 0; i < 255; i++)
        {
            for (int x = 0; x < 512; x++)
            {
                hsv_t hsv = {i, 255, 255};
                frame_buffer[x] = hsv_to_rgb(hsv);
            }
            vTaskDelay(20 / portTICK_PERIOD_MS);
        }
    }
}
