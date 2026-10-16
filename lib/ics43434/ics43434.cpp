#include "ics43434.h"

#include <driver/i2s_std.h>
#include <esp_dsp.h>
#include <esp_log.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define DELAY pdMS_TO_TICKS(10)

#define N_SAMPLES 1024
#define I2S_CHANNELS 2
#define I2S_READ_LEN (N_SAMPLES * I2S_CHANNELS * sizeof(int32_t))
#define I2S_SAMPLE_RATE 16000
#define I2S_NUM I2S_NUM_0

#define SPECTRUM_NOISE_MARGIN_DB 2.5f
#define SPECTRUM_DYNAMIC_RANGE_DB 18.0f

#define I2S_BCK_IO GPIO_NUM_4
#define I2S_WS_IO GPIO_NUM_2
#define I2S_DO_IO I2S_GPIO_UNUSED
#define I2S_DI_IO GPIO_NUM_15

static const char *TAG = "ics43434";
static bool initialized = false;
static TaskHandle_t i2s_adc_task_handle = nullptr;
static i2s_chan_handle_t i2s_rx_handle = nullptr;

static SemaphoreHandle_t spectrum_mutex = nullptr;
static float spectrum_bars[ICS43434_SPECTRUM_BARS] = {0};

static void i2s_adc_task(void *);

void ics43434_init() {
  if (initialized) return;

  i2s_chan_config_t channel_config =
      I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM, I2S_ROLE_MASTER);
  channel_config.dma_desc_num = 2;
  channel_config.dma_frame_num = N_SAMPLES;

  esp_err_t err =
      i2s_new_channel(&channel_config, nullptr, &i2s_rx_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed creating I2S channel: %s", esp_err_to_name(err));
    return;
  }

  i2s_std_config_t standard_config = {};
  standard_config.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE);
  standard_config.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
      I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO);
  standard_config.slot_cfg.slot_mask = I2S_STD_SLOT_BOTH;
  standard_config.gpio_cfg.mclk = I2S_GPIO_UNUSED;
  standard_config.gpio_cfg.bclk = I2S_BCK_IO;
  standard_config.gpio_cfg.ws = I2S_WS_IO;
  standard_config.gpio_cfg.dout = I2S_DO_IO;
  standard_config.gpio_cfg.din = I2S_DI_IO;

  err = i2s_channel_init_std_mode(i2s_rx_handle, &standard_config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed configuring I2S channel: %s", esp_err_to_name(err));
    i2s_del_channel(i2s_rx_handle);
    i2s_rx_handle = nullptr;
    return;
  }

  err = i2s_channel_enable(i2s_rx_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed enabling I2S channel: %s", esp_err_to_name(err));
    i2s_del_channel(i2s_rx_handle);
    i2s_rx_handle = nullptr;
    return;
  }

  spectrum_mutex = xSemaphoreCreateMutex();
  if (spectrum_mutex == nullptr) {
    ESP_LOGE(TAG, "Failed creating microphone mutex");
    i2s_channel_disable(i2s_rx_handle);
    i2s_del_channel(i2s_rx_handle);
    i2s_rx_handle = nullptr;
    return;
  }

  if (xTaskCreate(i2s_adc_task, "ics43434", 16384, nullptr, 1,
                  &i2s_adc_task_handle) != pdPASS) {
    ESP_LOGE(TAG, "Failed creating microphone task");
    vSemaphoreDelete(spectrum_mutex);
    spectrum_mutex = nullptr;
    i2s_channel_disable(i2s_rx_handle);
    i2s_del_channel(i2s_rx_handle);
    i2s_rx_handle = nullptr;
    return;
  }

  initialized = true;
}

bool ics43434_read_spectrum(float bars[ICS43434_SPECTRUM_BARS],
                            TickType_t timeout) {
  if (bars == nullptr || spectrum_mutex == nullptr ||
      xSemaphoreTake(spectrum_mutex, timeout) != pdTRUE) {
    return false;
  }

  memcpy(bars, spectrum_bars, sizeof(spectrum_bars));
  xSemaphoreGive(spectrum_mutex);
  return true;
}

static void i2s_adc_task(void *arg) {
  (void)arg;

  int32_t *i2s_frames = static_cast<int32_t *>(
      calloc(N_SAMPLES * I2S_CHANNELS, sizeof(int32_t)));
  float *window = static_cast<float *>(calloc(N_SAMPLES, sizeof(float)));
  float *fft = static_cast<float *>(calloc(N_SAMPLES * 2, sizeof(float)));
  if (i2s_frames == nullptr || window == nullptr || fft == nullptr) {
    ESP_LOGE(TAG, "Failed allocating FFT buffers");
    free(i2s_frames);
    free(window);
    free(fft);
    vTaskDelete(nullptr);
    return;
  }

  esp_err_t err = dsps_fft2r_init_fc32(nullptr, N_SAMPLES);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed initializing FFT: %s", esp_err_to_name(err));
    free(i2s_frames);
    free(window);
    free(fft);
    vTaskDelete(nullptr);
    return;
  }
  dsps_wind_hann_f32(window, N_SAMPLES);

  float noise_floor[ICS43434_SPECTRUM_BARS] = {0};
  bool noise_floor_initialized = false;

  for (;;) {
    size_t bytes_read = 0;
    err = i2s_channel_read(i2s_rx_handle, i2s_frames, I2S_READ_LEN, &bytes_read,
                           portMAX_DELAY);
    if (err != ESP_OK || bytes_read != I2S_READ_LEN) {
      ESP_LOGW(TAG, "I2S read failed: %s", esp_err_to_name(err));
      vTaskDelay(DELAY);
      continue;
    }

    float sample_mean = 0;
    for (int i = 0; i < N_SAMPLES; i++) {
      sample_mean +=
          static_cast<float>(i2s_frames[i * I2S_CHANNELS]) / 2147483648.0f;
    }
    sample_mean /= N_SAMPLES;

    for (int i = 0; i < N_SAMPLES; i++) {
      const float sample = static_cast<float>(
                               i2s_frames[i * I2S_CHANNELS]) /
                               2147483648.0f -
                           sample_mean;
      fft[i * 2] = sample * window[i];
      fft[i * 2 + 1] = 0;
    }

    dsps_fft2r_fc32(fft, N_SAMPLES);
    dsps_bit_rev_fc32(fft, N_SAMPLES);

    float next_bars[ICS43434_SPECTRUM_BARS] = {0};
    static const int band_edges[ICS43434_SPECTRUM_BARS + 1] = {
        22, 29, 37, 48, 62, 80, 104, 134, 173,
    };

    float band_db[ICS43434_SPECTRUM_BARS];
    float strongest_band_db = -120.0f;
    for (int band = 0; band < ICS43434_SPECTRUM_BARS; band++) {
      float band_power = 0;
      for (int bin = band_edges[band]; bin < band_edges[band + 1]; bin++) {
        const float real = fft[bin * 2];
        const float imag = fft[bin * 2 + 1];
        band_power += real * real + imag * imag;
      }

      // Convert single-sided Hann-windowed FFT energy back to mean-square
      // signal power. This keeps equal-amplitude tones comparable across bands.
      const float mean_square =
          band_power * (16.0f / (3.0f * N_SAMPLES * N_SAMPLES));
      band_db[band] =
          10.0f * log10f(fmaxf(mean_square, 1e-12f));
      strongest_band_db = fmaxf(strongest_band_db, band_db[band]);
    }

    for (int band = 0; band < ICS43434_SPECTRUM_BARS; band++) {
      const float decibels = band_db[band];
      if (!noise_floor_initialized) {
        noise_floor[band] = decibels;
      } else if (decibels < noise_floor[band]) {
        noise_floor[band] += (decibels - noise_floor[band]) * 0.10f;
      } else if (decibels < noise_floor[band] + 6.0f) {
        noise_floor[band] += (decibels - noise_floor[band]) * 0.002f;
      }

      const float signal_db =
          fmaxf(0.0f, decibels - noise_floor[band] - SPECTRUM_NOISE_MARGIN_DB);
      const float spectral_contrast_db = fmaxf(
          0.0f,
          SPECTRUM_DYNAMIC_RANGE_DB + decibels - strongest_band_db);
      const float visible_db = fminf(signal_db, spectral_contrast_db);
      const float normalized = visible_db / SPECTRUM_DYNAMIC_RANGE_DB;
      next_bars[band] = fminf(255.0f, fmaxf(0.0f, normalized * 255.0f));
    }
    noise_floor_initialized = true;

    if (xSemaphoreTake(spectrum_mutex, portMAX_DELAY) == pdTRUE) {
      memcpy(spectrum_bars, next_bars, sizeof(spectrum_bars));
      xSemaphoreGive(spectrum_mutex);
    }
    vTaskDelay(DELAY);
  }
}
