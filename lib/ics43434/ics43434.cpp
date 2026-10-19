#include "ics43434.h"

#include <driver/i2s_std.h>
#include <esp_dsp.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_DELAY pdMS_TO_TICKS(10)

#define N_SAMPLES 1024
#define HOP_SAMPLES 256
#define FFT_UPDATE_HOPS 2
#define I2S_CHANNELS 2
#define I2S_READ_LEN (HOP_SAMPLES * I2S_CHANNELS * sizeof(int32_t))
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
static float detected_bpm = 0;
static float bpm_confidence = 0;
static volatile bool microphone_muted = false;
static volatile bool spectrum_enabled = false;

static void i2s_adc_task(void *);

void ics43434_init() {
  if (initialized) return;

  i2s_chan_config_t channel_config =
      I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM, I2S_ROLE_MASTER);
  channel_config.dma_desc_num = 4;
  channel_config.dma_frame_num = HOP_SAMPLES;

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
  ESP_LOGI(TAG, "ICS-43434 capture and FFT task initialized");
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

void ics43434_set_muted(bool muted) {
  microphone_muted = muted;
  ESP_LOGI(TAG, "Microphone %s", muted ? "muted" : "unmuted");
}

bool ics43434_is_muted() { return microphone_muted; }

void ics43434_set_spectrum_enabled(bool enabled) {
  spectrum_enabled = enabled;
  if (!enabled && spectrum_mutex != nullptr &&
      xSemaphoreTake(spectrum_mutex, portMAX_DELAY) == pdTRUE) {
    memset(spectrum_bars, 0, sizeof(spectrum_bars));
    detected_bpm = 0;
    bpm_confidence = 0;
    xSemaphoreGive(spectrum_mutex);
  }
}

bool ics43434_read_bpm(float *bpm, float *confidence, TickType_t timeout) {
  if (bpm == nullptr || confidence == nullptr || spectrum_mutex == nullptr ||
      xSemaphoreTake(spectrum_mutex, timeout) != pdTRUE) {
    return false;
  }

  *bpm = detected_bpm;
  *confidence = bpm_confidence;
  xSemaphoreGive(spectrum_mutex);
  return true;
}

static void i2s_adc_task(void *arg) {
  (void)arg;

  int32_t *i2s_frames = static_cast<int32_t *>(
      calloc(HOP_SAMPLES * I2S_CHANNELS, sizeof(int32_t)));
  float *audio = static_cast<float *>(calloc(N_SAMPLES, sizeof(float)));
  float *window = static_cast<float *>(calloc(N_SAMPLES, sizeof(float)));
  float *fft = static_cast<float *>(calloc(N_SAMPLES * 2, sizeof(float)));
  if (i2s_frames == nullptr || audio == nullptr || window == nullptr ||
      fft == nullptr) {
    ESP_LOGE(TAG, "Failed allocating FFT buffers");
    free(i2s_frames);
    free(audio);
    free(window);
    free(fft);
    vTaskDelete(nullptr);
    return;
  }

  esp_err_t err = dsps_fft2r_init_fc32(nullptr, N_SAMPLES);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed initializing FFT: %s", esp_err_to_name(err));
    free(i2s_frames);
    free(audio);
    free(window);
    free(fft);
    vTaskDelete(nullptr);
    return;
  }
  dsps_wind_hann_f32(window, N_SAMPLES);

  float noise_floor[ICS43434_SPECTRUM_BARS] = {0};
  bool noise_floor_initialized = false;
  float previous_bass_db = -120.0f;
  float bass_average_db = -120.0f;
  float onset_average = 0;
  float onset_deviation = 0.5f;
  float beat_intervals[8] = {0};
  int beat_interval_count = 0;
  int beat_interval_index = 0;
  uint32_t last_beat_ms = 0;
  bool beat_detector_initialized = false;
  float next_bpm = 0;
  float next_bpm_confidence = 0;
  int buffered_samples = 0;
  int fft_hops = 0;

  for (;;) {
    size_t bytes_read = 0;
    err = i2s_channel_read(i2s_rx_handle, i2s_frames, I2S_READ_LEN, &bytes_read,
                           portMAX_DELAY);
    if (err != ESP_OK || bytes_read != I2S_READ_LEN) {
      ESP_LOGW(TAG, "I2S read failed: %s", esp_err_to_name(err));
      vTaskDelay(ERROR_DELAY);
      continue;
    }

    if (microphone_muted || !spectrum_enabled) {
      buffered_samples = 0;
      fft_hops = 0;
      beat_detector_initialized = false;
      beat_interval_count = 0;
      beat_interval_index = 0;
      last_beat_ms = 0;
      next_bpm = 0;
      next_bpm_confidence = 0;
      if (xSemaphoreTake(spectrum_mutex, portMAX_DELAY) == pdTRUE) {
        memset(spectrum_bars, 0, sizeof(spectrum_bars));
        detected_bpm = 0;
        bpm_confidence = 0;
        xSemaphoreGive(spectrum_mutex);
      }
      continue;
    }

    memmove(audio, audio + HOP_SAMPLES,
            (N_SAMPLES - HOP_SAMPLES) * sizeof(float));
    float hop_mean = 0;
    for (int i = 0; i < HOP_SAMPLES; i++) {
      const float sample =
          static_cast<float>(i2s_frames[i * I2S_CHANNELS]) / 2147483648.0f;
      audio[N_SAMPLES - HOP_SAMPLES + i] = sample;
      hop_mean += sample;
    }
    hop_mean /= HOP_SAMPLES;
    buffered_samples =
        buffered_samples + HOP_SAMPLES > N_SAMPLES
            ? N_SAMPLES
            : buffered_samples + HOP_SAMPLES;

    float hop_power = 0;
    for (int i = N_SAMPLES - HOP_SAMPLES; i < N_SAMPLES; i++) {
      const float centered = audio[i] - hop_mean;
      hop_power += centered * centered;
    }
    const float bass_mean_square = hop_power / HOP_SAMPLES;
    const float bass_db =
        10.0f * log10f(fmaxf(bass_mean_square, 1e-12f));
    const uint32_t now_ms =
        static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);

    if (!beat_detector_initialized) {
      previous_bass_db = bass_db;
      bass_average_db = bass_db;
      beat_detector_initialized = true;
    } else {
      const float onset = fmaxf(0.0f, bass_db - previous_bass_db);
      const float threshold =
          onset_average + fmaxf(1.5f, onset_deviation * 2.5f);
      const uint32_t since_last_beat = now_ms - last_beat_ms;
      const bool is_beat = onset > threshold &&
                           bass_db > bass_average_db + 2.0f &&
                           (last_beat_ms == 0 || since_last_beat >= 350);

      const float average_delta = onset - onset_average;
      onset_average += average_delta * 0.01f;
      onset_deviation +=
          (fabsf(average_delta) - onset_deviation) * 0.01f;
      bass_average_db +=
          (bass_db - bass_average_db) * (bass_db < bass_average_db ? 0.025f
                                                                    : 0.004f);
      previous_bass_db = bass_db;

      if (is_beat) {
        if (last_beat_ms != 0) {
          const float interval = now_ms - last_beat_ms;
          if (interval >= 250.0f && interval <= 1500.0f) {
            beat_intervals[beat_interval_index] = interval;
            beat_interval_index = (beat_interval_index + 1) % 8;
            if (beat_interval_count < 8) beat_interval_count++;
          }
        }
        last_beat_ms = now_ms;

        if (beat_interval_count >= 3) {
          float sorted[8];
          memcpy(sorted, beat_intervals,
                 beat_interval_count * sizeof(float));
          for (int i = 1; i < beat_interval_count; i++) {
            const float value = sorted[i];
            int j = i - 1;
            while (j >= 0 && sorted[j] > value) {
              sorted[j + 1] = sorted[j];
              j--;
            }
            sorted[j + 1] = value;
          }

          const float seed_period = sorted[beat_interval_count / 2];
          int consistent = 0;
          float period_sum = 0;
          const float tolerance = fmaxf(80.0f, seed_period * 0.18f);
          for (int i = 0; i < beat_interval_count; i++) {
            const float multiple =
                fmaxf(1.0f, roundf(beat_intervals[i] / seed_period));
            const float normalized_interval = beat_intervals[i] / multiple;
            if (fabsf(normalized_interval - seed_period) <= tolerance) {
              period_sum += normalized_interval;
              consistent++;
            }
          }

          float period =
              consistent > 0 ? period_sum / consistent : seed_period;
          while (period > 1000.0f) period *= 0.5f;
          while (period < 300.0f) period *= 2.0f;
          next_bpm = 60000.0f / period;
          next_bpm_confidence =
              (float)consistent / beat_interval_count *
              fminf(1.0f, beat_interval_count / 4.0f);
        }
      }

      if (last_beat_ms != 0 && now_ms - last_beat_ms > 2500) {
        next_bpm_confidence *= 0.98f;
        if (next_bpm_confidence < 0.10f) {
          next_bpm = 0;
          next_bpm_confidence = 0;
        }
      }
    }

    fft_hops++;
    if (buffered_samples < N_SAMPLES || fft_hops < FFT_UPDATE_HOPS) {
      if (xSemaphoreTake(spectrum_mutex, portMAX_DELAY) == pdTRUE) {
        detected_bpm = next_bpm;
        bpm_confidence = next_bpm_confidence;
        xSemaphoreGive(spectrum_mutex);
      }
      continue;
    }
    fft_hops = 0;

    float sample_mean = 0;
    for (int i = 0; i < N_SAMPLES; i++) sample_mean += audio[i];
    sample_mean /= N_SAMPLES;
    for (int i = 0; i < N_SAMPLES; i++) {
      fft[i * 2] = (audio[i] - sample_mean) * window[i];
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
      detected_bpm = next_bpm;
      bpm_confidence = next_bpm_confidence;
      xSemaphoreGive(spectrum_mutex);
    }
  }
}
