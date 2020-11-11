#if !defined(EFFECTS_H)
#define EFFECTS_H

#include <esp_event.h>
#include <freertos/task.h>

#include "event_loop.h"
#include "framebuffer.h"

void init_effects();

void cube_color_wheel();
void wave_color_wheel();
void mic_spectrum();
void shift_planes();
void rain();
void vertical_travelers();
void shift_suspend();
void wireframes();
void ripples();
void sidewaves();
void ball();
void test();
void fireworks();

// Utilities
float distance2d(float x1, float y1, float x2, float y2);
float distance3d (float x1, float y1, float z1, float x2, float y2, float z2);

extern esp_event_loop_handle_t effects_event_loop;
extern TaskHandle_t effect_loop_task_handle;

ESP_EVENT_DECLARE_BASE(EFFECT_EVENTS);

#endif  // EFFECTS_H
