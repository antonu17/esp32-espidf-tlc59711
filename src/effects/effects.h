#if !defined(EFFECTS_H)
#define EFFECTS_H

#include "esp_event.h"
#include "framebuffer.h"

void init_effects();

void cube_color_wheel();
void wave_color_wheel();
void mic_spectrum();

extern esp_event_loop_handle_t effects_event_loop;
ESP_EVENT_DECLARE_BASE(EFFECT_EVENTS);

#endif  // EFFECTS_H
