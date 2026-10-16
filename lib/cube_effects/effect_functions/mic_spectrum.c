#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>

#include "../../ics43434/ics43434.h"
#include "effect_functions.h"

#define FRAME_DELAY pdMS_TO_TICKS(40)
#define BAR_SCALE 32.0f

static float clampf(float value, float minimum, float maximum) {
  return fminf(maximum, fmaxf(minimum, value));
}

static rgb_t scale_color(rgb_t color, float brightness) {
  brightness = clampf(brightness, 0.0f, 1.0f);
  return (rgb_t){
      .r = (uint8_t)(color.r * brightness),
      .g = (uint8_t)(color.g * brightness),
      .b = (uint8_t)(color.b * brightness),
  };
}

static rgb_t highlight_color(rgb_t color) {
  return (rgb_t){
      .r = (uint8_t)(((uint16_t)color.r + 255) / 2),
      .g = (uint8_t)(((uint16_t)color.g + 255) / 2),
      .b = (uint8_t)(((uint16_t)color.b + 255) / 2),
  };
}

void mic_spectrum(effect_t *effect) {
  float smoothed[ICS43434_SPECTRUM_BARS] = {0};
  float peaks[ICS43434_SPECTRUM_BARS] = {0};
  uint8_t hue_phase = 0;

  ics43434_init();
  fb_clear();

  while (effect->running) {
    float bars[ICS43434_SPECTRUM_BARS];
    if (!ics43434_read_spectrum(bars, portMAX_DELAY)) continue;

    float levels[ICS43434_SPECTRUM_BARS];
    for (int band = 0; band < ICS43434_SPECTRUM_BARS; band++) {
      const float left = bars[band > 0 ? band - 1 : band];
      const float right =
          bars[band + 1 < ICS43434_SPECTRUM_BARS ? band + 1 : band];
      const float spectrum_smoothed =
          bars[band] * 0.70f + (left + right) * 0.15f;
      const float gated = fmaxf(0.0f, spectrum_smoothed / BAR_SCALE - 0.45f);
      const float target = clampf(gated * 1.06f, 0.0f, 8.0f);
      const float smoothing = target > smoothed[band] ? 0.45f : 0.12f;

      smoothed[band] += (target - smoothed[band]) * smoothing;
      if (smoothed[band] < 0.08f) smoothed[band] = 0;
      peaks[band] = fmaxf(smoothed[band], peaks[band] - 0.12f);
      levels[band] = smoothed[band];
    }

    fb_shift(FB_AXIS_X, FB_SHIFT_BACK);

    for (int band = 0; band < ICS43434_SPECTRUM_BARS; band++) {
      for (int z = 0; z < 8; z++) {
        const float coverage = clampf(levels[band] - z, 0.0f, 1.0f);
        if (coverage <= 0) continue;

        const uint8_t hue = hue_phase + band * 25 - z * 4;
        const float height_brightness = 0.25f + 0.75f * (z + 1) / 8.0f;
        const rgb_t color = scale_color(hue_to_rgb_sine2(hue),
                                        coverage * height_brightness);
        fb_set_pixel(7, band, z, color);
      }

      if (peaks[band] > levels[band] + 0.35f) {
        const int peak_z = (int)clampf(floorf(peaks[band]), 0.0f, 7.0f);
        const uint8_t hue = hue_phase + band * 25 - peak_z * 4;
        fb_set_pixel(7, band, peak_z,
                     highlight_color(hue_to_rgb_sine2(hue)));
      }
    }

    hue_phase += 2;
    vTaskDelay(FRAME_DELAY);
  }

  fb_clear();
}
