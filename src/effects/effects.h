#if !defined(EFFECTS_H)
#define EFFECTS_H

#include <esp_event.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "event_loop.h"
#include "framebuffer.h"

#define EFFECT_TIMEOUT_BIT 0x01
#define EFFECT_STOP_BIT 0x02

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

extern esp_event_loop_handle_t effects_event_loop;
extern TaskHandle_t effect_loop_task_handle;

ESP_EVENT_DECLARE_BASE(EFFECT_EVENTS);

#endif  // EFFECTS_H
