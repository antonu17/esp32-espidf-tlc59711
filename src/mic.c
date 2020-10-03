/*
 * Copyright 2019 by Morgan Allen
 *
 * This work is licensed under a Creative Commons Attribution-NonCommercial 4.0 International
 * https://creativecommons.org/licenses/by-nc/4.0/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/queue.h"
#include "driver/i2s.h"
#include "driver/gpio.h"

#define TAG "I2teSt"

#define I2S_SAMPLE_RATE   (44100)
#define I2S_NUM           (I2S_NUM_0)
#define I2S_READ_LEN      (1 << 7)
#define I2S_BCK_IO        (GPIO_NUM_4)
#define I2S_WS_IO         (GPIO_NUM_2)
#define I2S_DO_IO         (-1)
#define I2S_DI_IO         (GPIO_NUM_15)

#define GPIO_INPUT_PIN_SEL    ((1ULL<<I2S_DI_IO))
#define GPIO_OUTPUT_PIN_SEL   ((1ULL << I2S_BCK_IO) | (1ULL << I2S_WS_IO))

static char* i2s_read_buff;

void i2s_init() {
  esp_err_t err;

  // unsure if setting the IOs is needed
  gpio_config_t io_conf;

  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  io_conf.pull_up_en = 0;
  io_conf.pull_down_en = 0;

  gpio_config(&io_conf);

  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 0;

  gpio_config(&io_conf);

  // very straigh forward, cut and paste setup
  // this was taken from https://github.com/maspetsberger/esp32-i2s-mems/blob/master/examples/VUMeterDemo/VUMeterDemo.ino
  // and when compiled under Arduino ESP32 1.0.2 runs as expected.
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_24BIT,
    .channel_format = I2S_CHANNEL_FMT_ALL_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = I2S_READ_LEN
  };

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCK_IO,
    .ws_io_num = I2S_WS_IO,
    .data_out_num = I2S_DO_IO,
    .data_in_num = I2S_DI_IO
  };

  err = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    ESP_LOGI(TAG, "Failed installing driver: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_NUM, &pin_config);
  if (err != ESP_OK) {
    ESP_LOGI(TAG, "Failed setting pin: %d\n", err);
    while (true);
  }
  err = i2s_set_sample_rates(I2S_NUM, I2S_SAMPLE_RATE);
  if (err != ESP_OK) {
    ESP_LOGI(TAG, "Failed setting sample rate: %d\n", err);
    while (true);
  }
  ESP_LOGI(TAG, "I2S driver installed.");
}

void init_mic() {
  i2s_init();

  size_t bytes_read;

  i2s_read_buff = (char*) calloc(I2S_READ_LEN, sizeof(char));

  // dont know if needed, doesn't work either way
  i2s_start(I2S_NUM);

  while(1) {
    // no errors are thrown
    ESP_ERROR_CHECK(i2s_read(I2S_NUM, (void*) i2s_read_buff, I2S_READ_LEN, &bytes_read, portMAX_DELAY));

    // read the correct number of bytes
    ESP_LOGI(TAG, "read %d bytes", bytes_read);

    for(int i = 0; i < bytes_read; i++) {
      // all 0 output
      ESP_LOGI(TAG, "%d", (int)i2s_read_buff[i]);
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
};
