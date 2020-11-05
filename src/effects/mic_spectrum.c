#include <driver/gpio.h>
#include <driver/i2s.h>
#include <esp_event.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>

#include "effects.h"
#include "esp_dsp.h"

#define TAG __FILE__
#define DELAY pdMS_TO_TICKS(10)

#define N_SAMPLES 1024
#define I2S_SAMPLE_RATE (16000)
#define I2S_SAMPLE_BITS (I2S_BITS_PER_SAMPLE_16BIT)
#define I2S_NUM (I2S_NUM_0)
#define I2S_READ_LEN (2 * N_SAMPLES)

#define I2S_BCK_IO (GPIO_NUM_4)
#define I2S_WS_IO (GPIO_NUM_2)
#define I2S_DO_IO (-1)
#define I2S_DI_IO (GPIO_NUM_15)

TaskHandle_t i2s_adc_task_handle = NULL;
static int running;

void i2s_init() {
    esp_err_t err;

    const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_SAMPLE_BITS,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 2,
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

void i2s_deinit() {
    ESP_ERROR_CHECK(i2s_driver_uninstall(I2S_NUM));
}

void i2s_adc_task(void* arg) {
    int i2s_read_len = I2S_READ_LEN;
    int N = N_SAMPLES;
    size_t bytes_read;
    char* i2s_read_buff = (char*)calloc(i2s_read_len, sizeof(char));
    int16_t* samples_sc16 = (int16_t*)i2s_read_buff;
    float* samples_fc32 = (float*)calloc(N, sizeof(float));

    uint8_t data[8] = {0};
    float data_f32[8] = {0};
    float wind[N];
    float y_cf[N * 2];
    // Pointers to result array
    float* y1_cf = &y_cf[0];
    // float* y2_cf = &y_cf[N];

    esp_err_t ret;
    ret = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Not possible to initialize FFT. Error = %i", ret);
        return;
    }
    // Window coefficients
    dsps_wind_hann_f32(wind, N);

    while (1) {
        float positive_sampels_count = 0;
        float positive_sampels_sum = 0;
        float positive_sampels_avg = 0;

        i2s_read(I2S_NUM, (void*)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
        for (int i = 0; i < N_SAMPLES; i++) {
            float sample = ((float)samples_sc16[i]) / 10000;  //32768
            samples_fc32[i] = sample;
            if (sample > 0) {
                positive_sampels_count++;
                positive_sampels_sum += sample;
            }
        }
        positive_sampels_avg = positive_sampels_sum / positive_sampels_count;

        // Convert two input vectors to one complex vector
        for (int i = 0; i < N; i++) {
            y_cf[i * 2 + 0] = samples_fc32[i] * wind[i];
            y_cf[i * 2 + 1] = 0;
        }

        dsps_fft2r_fc32(y_cf, N);
        dsps_bit_rev_fc32(y_cf, N);
        dsps_cplx2reC_fc32(y_cf, N);
        for (int i = 0; i < N / 2; i++) {
            y1_cf[i] = 10 * log10f((y1_cf[i * 2 + 0] * y1_cf[i * 2 + 0] + y1_cf[i * 2 + 1] * y1_cf[i * 2 + 1]) / N);
            // y2_cf[i] = 10 * log10f((y2_cf[i * 2 + 0] * y2_cf[i * 2 + 0] + y2_cf[i * 2 + 1] * y2_cf[i * 2 + 1])/N);
        }

        int bin_size = (N / 2) / 8;
        memset(data_f32, 0, 8 * sizeof(float));
        memset(data, 0, 8 * sizeof(uint8_t));
        for (int i = 0; i < N / 2; i++) {
            int bin = i / bin_size;
            data_f32[bin] = fmax(data_f32[bin], fabs(y1_cf[i]));
        }
        for (int i = 0; i < 8; i++) {
            data_f32[i] *= positive_sampels_sum;
            data[i] = (uint8_t)data_f32[i];
        }

        ESP_ERROR_CHECK(esp_event_post_to(event_loop, MIC_EVENTS, MIC_EVENT_FRAME_READY, data, 8 * sizeof(uint8_t), portMAX_DELAY));
        vTaskDelay(DELAY);
    }
    free(i2s_read_buff);
    i2s_read_buff = NULL;
    vTaskDelete(NULL);
}

void mic_frame_ready_handler(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data) {
    uint8_t* s = ((uint8_t*)event_data);
    rgb_t rgb;
    uint8_t c = 0;
    for (int i = 0; i < 8; i++) {
        s[i] = s[i] / 32;
    }

    fb_shift(FB_AXIS_X, FB_SHIFT_BACK);

    for (int x = 0; x < 8; x++) {
        int8_t v = s[x];
        rgb = hue_to_rgb_sine2(c);
        // while (v >= 0) {
        //     fb_set_pixel(7, x, 7 - v--, rgb);
        // }
        fb_set_pixel(7, x, 7 - v, rgb);
        c += 32;
    }
}

void mic_spectrum_stop() {
    ESP_LOGI(TAG, "stop event received");
    if (i2s_adc_task_handle != NULL) {
        vTaskDelete(i2s_adc_task_handle);
    }
    i2s_deinit();
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, MIC_EVENTS, MIC_EVENT_FRAME_READY, mic_frame_ready_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, mic_spectrum_stop));
    running = 0;
}

void mic_spectrum() {
    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, MIC_EVENTS, MIC_EVENT_FRAME_READY, mic_frame_ready_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register_with(event_loop, EFFECT_EVENTS, EFFECT_EVENT_STOP, mic_spectrum_stop, NULL));
    running = 1;

    i2s_init();
    xTaskCreatePinnedToCore(i2s_adc_task, "i2s_adc_task", 1024 * 16, NULL, 1, &i2s_adc_task_handle, 1);

    fb_clear();
    while (running) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    fb_clear();

    ESP_LOGI(TAG, "notify effect_loop");
    xTaskNotify(effect_loop_task_handle, 0, eNoAction);
    vTaskDelay(portMAX_DELAY);
}
