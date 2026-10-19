#include "rgb-cube.h"

#include <esp_log.h>
#include <esp_timer.h>
#include <nvs.h>

#include <vector>
#include <cstring>

#include "../cube_framebuffer/draw.h"
#include "../cube_effects/effect_functions/effect_functions.h"
#include "../cube_effects/effect.h"
#include "../cube_framebuffer/framebuffer.h"
#include "../ics43434/ics43434.h"
#include "../tlc59711/tlc59711.h"

std::vector<effect_t *> effects = std::vector<effect_t *>();
static nvs_handle_t preferences = 0;

static bool initialized = false, running = false;
static TaskHandle_t effect_loop_task_h, effect_task_h;
static SemaphoreHandle_t effect_task_semaphore, cube_stop_semaphore;
static uint32_t effect_timeout_ms = 0;
static size_t running_effect_idx = 0;
static const char *RUNNING_EFFECT_IDX = "key0";
static bool auto_cycle = true;
static bool ota_active = false;
static bool ota_restore_running = false;
static uint8_t cube_brightness = 10;

void effect_loop_task(void *);
void effect_task(void *);

static uint32_t millis_since_boot() {
  return static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
}

static void save_effect_index() {
  if (preferences == 0) return;
  if (nvs_set_u32(preferences, RUNNING_EFFECT_IDX, running_effect_idx) ==
      ESP_OK) {
    nvs_commit(preferences);
  }
}

void cube_init() {
  if (initialized) return;
  initialized = true;

  effects.push_back(effect_new("fireworks", fireworks, NULL));
  effects.push_back(effect_new("sidewaves", sidewaves, NULL));
  effects.push_back(effect_new("ball", ball, NULL));
  effects.push_back(effect_new("shift_suspend", shift_suspend, NULL));
  effects.push_back(effect_new("cube_color_wheel", cube_color_wheel, NULL));
  effects.push_back(effect_new("linespin", linespin, NULL));
  effects.push_back(effect_new("rain", rain, NULL));
  effects.push_back(effect_new("ripples", ripples, NULL));
  effects.push_back(effect_new("shift_planes", shift_planes, NULL));
  effects.push_back(effect_new("sinelines", sinelines, NULL));
  effects.push_back(effect_new("vertical_travelers", vertical_travelers, NULL));
  effects.push_back(effect_new("wave_color_wheel", wave_color_wheel, NULL));
  effects.push_back(effect_new("wireframes", wireframes, NULL));
  effects.push_back(
      effect_new("mic_spectrum", mic_spectrum, mic_spectrum_stop));

  uint32_t saved_effect_idx = 0;
  if (nvs_open("rgbcube", NVS_READWRITE, &preferences) == ESP_OK) {
    nvs_get_u32(preferences, RUNNING_EFFECT_IDX, &saved_effect_idx);
  }
  running_effect_idx = saved_effect_idx;
  if (running_effect_idx >= effects.size()) {
    running_effect_idx = effects.size() - 1;
    save_effect_index();
  }

  effect_task_semaphore = xSemaphoreCreateBinary();
  cube_stop_semaphore = xSemaphoreCreateBinary();
  init_framebuffer();
  ics43434_init();

  ESP_LOGI("rgb_cube", "Cube initialized with %u effects",
           static_cast<unsigned>(effects.size()));
}

void cube_start() {
  cube_init();
  if (running || ota_active) return;
  running = true;

  xTaskCreate(effect_loop_task, "effect_loop_task", 2000, NULL,
              tskIDLE_PRIORITY, &effect_loop_task_h);
  ESP_LOGD(__FILE__, "effect_loop_task started");

  start_framebuffer();
  ESP_LOGD(__FILE__, "start_framebuffer()");
}

void cube_stop() {
  if (!running) return;
  running = false;

  xSemaphoreGive(effect_task_semaphore);

  // Wait for effect_loop_task to shutdown
  xSemaphoreTake(cube_stop_semaphore, pdMS_TO_TICKS(1000));
  vTaskDelete(effect_loop_task_h);

  fb_clear();
  vTaskDelay(1);
  stop_framebuffer();
  ESP_LOGD(__FILE__, "stop_framebuffer()");
}

void _next_effect() {
  if (running_effect_idx >= (effects.size() - 1))
    running_effect_idx = 0;
  else
    running_effect_idx += 1;
  save_effect_index();
}

void _prev_effect() {
  if (running_effect_idx == 0)
    running_effect_idx = (effects.size() - 1);
  else
    running_effect_idx -= 1;
  save_effect_index();
}

void cube_next_effect() {
  _next_effect();
  if (running) xSemaphoreGive(effect_task_semaphore);
}

void cube_prev_effect() {
  _prev_effect();
  if (running) xSemaphoreGive(effect_task_semaphore);
}

bool cube_set_effect(const char *name) {
  if (name == nullptr) return false;
  cube_init();
  for (size_t i = 0; i < effects.size(); i++) {
    if (strcmp(effects.at(i)->name, name) == 0) {
      running_effect_idx = i;
      save_effect_index();
      if (running) xSemaphoreGive(effect_task_semaphore);
      return true;
    }
  }
  return false;
}

const char *cube_get_effect() {
  cube_init();
  return effects.at(running_effect_idx)->name;
}

void cube_set_mode(const char *mode) {
  if (mode == nullptr) return;

  auto_cycle = strcmp(mode, "Solo") != 0;
  cube_start();
}

const char *cube_get_mode() {
  return auto_cycle ? "Loop" : "Solo";
}

static void draw_ota_icon() {
  const rgb_t arrow = {0, 190, 255};
  const rgb_t tray = {0, 90, 255};
  const int z = 3;

  fb_clear_draft();
  for (int y = 0; y <= 3; y++) {
    fb_set_pixel_draft(3, y, z, arrow);
    fb_set_pixel_draft(4, y, z, arrow);
  }
  fb_set_pixel_draft(1, 3, z, arrow);
  fb_set_pixel_draft(2, 4, z, arrow);
  fb_set_pixel_draft(3, 5, z, arrow);
  fb_set_pixel_draft(4, 5, z, arrow);
  fb_set_pixel_draft(5, 4, z, arrow);
  fb_set_pixel_draft(6, 3, z, arrow);

  for (int x = 1; x <= 6; x++) {
    fb_set_pixel_draft(x, 7, z, tray);
  }
  fb_set_pixel_draft(1, 6, z, tray);
  fb_set_pixel_draft(6, 6, z, tray);
  fb_draw_draft();
}

void cube_ota_begin() {
  if (ota_active) return;

  cube_init();
  ota_restore_running = running;
  cube_stop();
  ota_active = true;
  cube_set_brightness(cube_brightness);
  draw_ota_icon();
  show_static_framebuffer_layer(3);
}

void cube_ota_abort() {
  if (!ota_active) return;

  fb_clear();
  stop_framebuffer();
  ota_active = false;
  cube_set_brightness(cube_brightness);
  if (ota_restore_running) cube_start();
}


void cube_effect_timeout(uint32_t sec) {
  ESP_LOGD(__FILE__, "set effect timeout: %d", sec);
  effect_timeout_ms = pdMS_TO_TICKS(sec * 1000);
}

void cube_set_brightness(uint8_t x) {
  cube_brightness = x;
  const uint8_t applied = ota_active && x > 24 ? 24 : x;
  tlc_set_brightness(applied, applied, applied);
}

void cube_set_microphone_muted(bool muted) {
  ics43434_set_muted(muted);
}

void effect_loop_task(void *arg) {
  while (running) {
    for (;;) {
      effect_t *effect = effects.at(running_effect_idx);
      ESP_LOGD(__FILE__, "start effect: %s", effect->name);
      effect->running = 1;
      xTaskCreate(effect_task, "effect_task", 4000, effect, 15,
                  &effect_task_h);

      uint32_t delta, start = millis_since_boot();
      for (;;) {
        if (pdTRUE ==
            xSemaphoreTake(effect_task_semaphore, pdMS_TO_TICKS(1000))) {
          break;
        }
        if (!auto_cycle || effect_timeout_ms == 0) {
          start = millis_since_boot();
          continue;
        }
        delta = millis_since_boot() - start;
        if (delta >= effect_timeout_ms) {
          _next_effect();
          break;
        }
      }
      effect->running = 0;
      vTaskDelete(effect_task_h);
      if (effect->stop_hook) effect->stop_hook(effect);
      if (!running) break;
    }
  }
  xSemaphoreGive(cube_stop_semaphore);
  vTaskDelay(portMAX_DELAY);
}

void effect_task(void *arg) {
  effect_t *effect = (effect_t *)arg;
  effect->function(effect);
  if (auto_cycle && effect_timeout_ms) _next_effect();
  xSemaphoreGive(effect_task_semaphore);
  vTaskDelay(portMAX_DELAY);
}
