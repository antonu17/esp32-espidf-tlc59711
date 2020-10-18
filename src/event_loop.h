#if !defined(EVENT_LOOP_H)
#define EVENT_LOOP_H

#include "esp_event.h"

extern esp_event_loop_handle_t event_loop;

ESP_EVENT_DECLARE_BASE(EFFECT_EVENTS);
ESP_EVENT_DECLARE_BASE(MIC_EVENTS);

enum {
    MIC_EVENT_FRAME_READY,
    EFFECT_EVENT_STOP,
};

void init_event_loop();

#endif  // EVENT_LOOP_H
