/*
 * Copyright 2019 by Morgan Allen
 *
 * This work is licensed under a Creative Commons Attribution-NonCommercial 4.0 International
 * https://creativecommons.org/licenses/by-nc/4.0/
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/i2s.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "spiffs.h"

#define TAG "MIC"

#define I2S_SAMPLE_RATE (16000)
#define I2S_SAMPLE_BITS (I2S_BITS_PER_SAMPLE_16BIT)
#define I2S_NUM (I2S_NUM_0)
#define I2S_READ_LEN (1 * 1024)

#define I2S_BCK_IO (GPIO_NUM_4)
#define I2S_WS_IO (GPIO_NUM_2)
#define I2S_DO_IO (-1)
#define I2S_DI_IO (GPIO_NUM_15)

#define I2S_CHANNEL_NUM (1)
#define RECORD_TIME (5)
#define FLASH_RECORD_SIZE (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORD_TIME)

const char filename[] = "/spiffs/recording.wav";
const int headerSize = 44;

void i2s_init() {
    esp_err_t err;

    const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_SAMPLE_BITS,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 64,
        .dma_buf_len = 1024,
        .use_apll = 1,
    };

    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_IO,
        .ws_io_num = I2S_WS_IO,
        .data_out_num = I2S_DO_IO,
        .data_in_num = I2S_DI_IO,
    };

    err = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Failed installing driver: %d\n", err);
    }

    err = i2s_set_pin(I2S_NUM, &pin_config);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Failed setting pin: %d\n", err);
    }

    err = i2s_set_sample_rates(I2S_NUM, I2S_SAMPLE_RATE);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Failed setting sample rate: %d\n", err);
    }

    ESP_LOGI(TAG, "I2S driver installed.");
}

void disp_buf(uint8_t* buf, int length) {
    printf("======\n");
    for (int i = 0; i < length; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }
    printf("======\n");
}

void wavHeader(uint8_t* header, int wavSize) {
    header[0] = 'R';
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';
    unsigned int fileSize = wavSize + headerSize - 8;
    header[4] = (uint8_t)(fileSize & 0xFF);
    header[5] = (uint8_t)((fileSize >> 8) & 0xFF);
    header[6] = (uint8_t)((fileSize >> 16) & 0xFF);
    header[7] = (uint8_t)((fileSize >> 24) & 0xFF);
    header[8] = 'W';
    header[9] = 'A';
    header[10] = 'V';
    header[11] = 'E';
    header[12] = 'f';
    header[13] = 'm';
    header[14] = 't';
    header[15] = ' ';
    header[16] = 0x10;
    header[17] = 0x00;
    header[18] = 0x00;
    header[19] = 0x00;
    header[20] = 0x01;
    header[21] = 0x00;
    header[22] = 0x01;
    header[23] = 0x00;
    header[24] = 0x80;
    header[25] = 0x3E;
    header[26] = 0x00;
    header[27] = 0x00;
    header[28] = 0x00;
    header[29] = 0x7D;
    header[30] = 0x00;
    header[31] = 0x00;
    header[32] = 0x02;
    header[33] = 0x00;
    header[34] = 0x10;
    header[35] = 0x00;
    header[36] = 'd';
    header[37] = 'a';
    header[38] = 't';
    header[39] = 'a';
    header[40] = (uint8_t)(wavSize & 0xFF);
    header[41] = (uint8_t)((wavSize >> 8) & 0xFF);
    header[42] = (uint8_t)((wavSize >> 16) & 0xFF);
    header[43] = (uint8_t)((wavSize >> 24) & 0xFF);
}

void i2s_adc(void* arg) {
    int i2s_read_len = I2S_READ_LEN;
    size_t bytes_read;
    char* i2s_read_buff = (char*)calloc(i2s_read_len, sizeof(char));

    // int flash_wr_size = 0;
    // uint8_t* flash_write_buff = (uint8_t*) calloc(i2s_read_len, sizeof(char));

    // FILE* f = fopen(filename, "w");
    // if (f == NULL) {
    //     ESP_LOGE(TAG, "Failed to open file for writing");
    //     return;
    // }

    // uint8_t header[headerSize];
    // wavHeader(header, FLASH_RECORD_SIZE);
    // fwrite(header, headerSize, 1, f);

    // while (flash_wr_size < FLASH_RECORD_SIZE) {
    while (1) {
        //read data from I2S bus, in this case, from ADC.
        i2s_read(I2S_NUM, (void*)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
        // disp_buf((uint8_t*)i2s_read_buff, 64);
        //save original data from I2S(ADC) into flash.
        // fwrite(i2s_read_buff, bytes_read, 1, f);
        // flash_wr_size += i2s_read_len;
        // ets_printf("Sound recording %u%%\n", flash_wr_size * 100 / FLASH_RECORD_SIZE);
        // ets_printf("Unused stack %u\n", uxTaskGetStackHighWaterMark(NULL));
    }
    free(i2s_read_buff);
    i2s_read_buff = NULL;

    // fclose(f);
    // ESP_LOGI(TAG, "File written");
    // list_dir(BASE_PATH);

    // ets_printf("Unused stack %u\n", uxTaskGetStackHighWaterMark(NULL));
    vTaskDelete(NULL);
}

void init_mic() {
    i2s_init();
    xTaskCreate(i2s_adc, "i2s_adc", 1024 * 2, NULL, 5, NULL);
};
