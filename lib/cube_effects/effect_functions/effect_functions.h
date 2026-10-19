#if !defined(CUBE_EFFECT_FUNCTIONS_H)
#define CUBE_EFFECT_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "../../cube_framebuffer/draw.h"
#include "../effect.h"

void cube_color_wheel(effect_t *effect);
void wave_color_wheel(effect_t *effect);
void shift_planes(effect_t *effect);
void rain(effect_t *effect);
void vertical_travelers(effect_t *effect);
void shift_suspend(effect_t *effect);
void wireframes(effect_t *effect);
void ripples(effect_t *effect);
void sidewaves(effect_t *effect);
void ball(effect_t *effect);
void fireworks(effect_t *effect);
void sinelines(effect_t *effect);
void linespin(effect_t *effect);
void mic_spectrum(effect_t *effect);
void mic_spectrum_stop(effect_t *effect);

#ifdef __cplusplus
}
#endif

#endif  // CUBE_EFFECT_FUNCTIONS_H
