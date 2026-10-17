#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>

#include "../../ics43434/ics43434.h"
#include "effect_functions.h"

#define FRAME_DELAY pdMS_TO_TICKS(40)
#define BAR_SCALE 32.0f
#define TWO_PI 6.28318530718f
#define CYLINDER_TWIST 0.09f

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

static float wrap_angle(float angle) {
  while (angle < 0) angle += TWO_PI;
  while (angle >= TWO_PI) angle -= TWO_PI;
  return angle;
}

static float cube_boundary_radius(float angle) {
  const float direction =
      fmaxf(fabsf(cosf(angle)), fabsf(sinf(angle)));
  return 3.5f / direction;
}

static float spectrum_shell_brightness(float normalized_radius) {
  static const float shell_radii[] = {0.18f, 0.45f, 0.72f, 0.99f};
  float brightness = 0;
  for (int i = 0; i < 4; i++) {
    const float shell = clampf(
        1.0f - fabsf(normalized_radius - shell_radii[i]) / 0.16f,
        0.0f, 1.0f);
    brightness = fmaxf(brightness, shell);
  }
  return brightness;
}

static void draw_runner(float runner_rotation, float rotation, float breath,
                        uint8_t hue_phase) {
  const float direction_x = cosf(runner_rotation);
  const float direction_y = sinf(runner_rotation);
  const float ray_length = cube_boundary_radius(runner_rotation);

  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      const float dx = x - 3.5f;
      const float dy = y - 3.5f;
      const float along = dx * direction_x + dy * direction_y;
      if (along < -0.45f) continue;

      const float progress = clampf(along / ray_length, 0.0f, 1.0f);
      const float across = fabsf(-dx * direction_y + dy * direction_x);
      const float width =
          clampf(1.0f - across / 0.68f, 0.0f, 1.0f);
      const float origin = clampf(along + 0.55f, 0.0f, 1.0f);
      const float tip = clampf(ray_length - along + 0.45f, 0.0f, 1.0f);
      const float radial_brightness =
          0.70f + 0.30f * progress;

      for (int z = 0; z < 8; z++) {
        const rgb_t current = fb_get_pixel_draft(x, y, z);
        if (z > 0 && current.r == 0 && current.g == 0 && current.b == 0) {
          continue;
        }

        const float brightness =
            width * origin * tip * radial_brightness * breath;
        if (brightness <= 0.015f) continue;

        const float color_angle = wrap_angle(
            runner_rotation - rotation - z * CYLINDER_TWIST);
        const float band_position =
            color_angle * ICS43434_SPECTRUM_BARS / TWO_PI;
        const uint8_t hue =
            hue_phase + (uint8_t)(band_position * 32.0f) - z * 3;
        const rgb_t runner =
            scale_color(hue_to_rgb_sine2(hue), brightness);
        const rgb_t combined = {
            .r = (uint8_t)fminf(255.0f, current.r + runner.r),
            .g = (uint8_t)fminf(255.0f, current.g + runner.g),
            .b = (uint8_t)fminf(255.0f, current.b + runner.b),
        };
        fb_set_pixel_draft(x, y, z, combined);
      }
    }
  }
}

void mic_spectrum(effect_t *effect) {
  float smoothed[ICS43434_SPECTRUM_BARS] = {0};
  float peaks[ICS43434_SPECTRUM_BARS] = {0};
  uint8_t peak_x[ICS43434_SPECTRUM_BARS] = {0};
  uint8_t peak_y[ICS43434_SPECTRUM_BARS] = {0};
  rgb_t peak_color[ICS43434_SPECTRUM_BARS] = {0};
  bool peak_position_valid[ICS43434_SPECTRUM_BARS] = {false};
  float rotation = 0;
  float runner_rotation = 0;
  float breath_phase = 0;
  uint8_t hue_phase = 0;

  ics43434_init();
  fb_clear();

  while (effect->running) {
    float bars[ICS43434_SPECTRUM_BARS];
    if (!ics43434_read_spectrum(bars, portMAX_DELAY)) continue;

    float levels[ICS43434_SPECTRUM_BARS];
    bool reposition_peak[ICS43434_SPECTRUM_BARS] = {false};
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
      if (smoothed[band] > peaks[band] + 0.05f) {
        peaks[band] = smoothed[band];
        reposition_peak[band] = true;
      } else {
        peaks[band] = fmaxf(smoothed[band], peaks[band] - 0.24f);
      }
      levels[band] = smoothed[band];
    }

    float energy = 0;
    for (int band = 0; band < ICS43434_SPECTRUM_BARS; band++) {
      energy += levels[band];
    }
    energy = clampf(energy / (ICS43434_SPECTRUM_BARS * 8.0f), 0.0f, 1.0f);

    const float peak_shell_radius = 0.99f;
    for (int band = 0; band < ICS43434_SPECTRUM_BARS; band++) {
      if (!reposition_peak[band]) continue;

      const int peak_z = (int)clampf(floorf(peaks[band]), 0.0f, 7.0f);
      const float angle =
          rotation + peak_z * CYLINDER_TWIST +
          band * TWO_PI / ICS43434_SPECTRUM_BARS;
      const float peak_radius =
          peak_shell_radius * cube_boundary_radius(angle);
      peak_x[band] = (uint8_t)lroundf(3.5f + peak_radius * cosf(angle));
      peak_y[band] = (uint8_t)lroundf(3.5f + peak_radius * sinf(angle));
      peak_color[band] =
          hue_to_rgb_sine2(hue_phase + band * 32 - peak_z * 3);
      peak_position_valid[band] = true;
    }

    fb_clear_draft();

    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {
        const float dx = x - 3.5f;
        const float dy = y - 3.5f;
        const float pixel_radius = sqrtf(dx * dx + dy * dy);
        const float pixel_angle = atan2f(dy, dx);
        const float normalized_radius =
            pixel_radius / cube_boundary_radius(pixel_angle);
        const float shell = spectrum_shell_brightness(normalized_radius);
        if (shell <= 0.05f) continue;

        for (int z = 0; z < 8; z++) {
          const float angle =
              wrap_angle(pixel_angle - rotation - z * CYLINDER_TWIST);
          const float band_position =
              angle * ICS43434_SPECTRUM_BARS / TWO_PI;
          const int band = (int)floorf(band_position);
          const int next_band = (band + 1) % ICS43434_SPECTRUM_BARS;
          const float mix = band_position - band;
          const float height =
              levels[band] * (1.0f - mix) + levels[next_band] * mix;
          const float coverage = clampf(height - z, 0.0f, 1.0f);
          if (coverage <= 0) continue;

          const uint8_t hue =
              hue_phase + (uint8_t)(band_position * 32.0f) - z * 3;
          const float height_brightness = 0.30f + 0.70f * (z + 1) / 8.0f;
          const rgb_t color = scale_color(
              hue_to_rgb_sine2(hue),
              shell * coverage * height_brightness * 0.72f);
          fb_set_pixel_draft(x, y, z, color);
        }
      }
    }

    const float breath = 0.34f + 0.18f * (0.5f + 0.5f * sinf(breath_phase));
    draw_runner(runner_rotation, rotation, breath, hue_phase);

    for (int band = 0; band < ICS43434_SPECTRUM_BARS; band++) {
      if (peak_position_valid[band] &&
          peaks[band] > levels[band] + 0.35f) {
        const int peak_z = (int)clampf(floorf(peaks[band]), 0.0f, 7.0f);
        fb_set_pixel_draft(peak_x[band], peak_y[band], peak_z,
                           peak_color[band]);
      }
    }

    fb_draw_draft();
    const float spin_step = 0.045f + energy * 0.055f;
    rotation = wrap_angle(rotation + spin_step);
    runner_rotation = wrap_angle(runner_rotation + spin_step * 3.0f);
    breath_phase = wrap_angle(breath_phase + 0.10f);
    hue_phase++;
    vTaskDelay(FRAME_DELAY);
  }

  fb_clear();
}
