#if !defined(EFFECTS_H)
#define EFFECTS_H

#include <esp_event.h>
#include <freertos/queue.h>

#include "event_loop.h"
#include "framebuffer.h"

void init_effects();

void cube_color_wheel();
void wave_color_wheel();
void mic_spectrum();
void shift_planes();
void rain();
void vertical_travelers();

extern esp_event_loop_handle_t effects_event_loop;
extern QueueHandle_t effects_queue;

ESP_EVENT_DECLARE_BASE(EFFECT_EVENTS);

#endif  // EFFECTS_H
