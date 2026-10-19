#if !defined(RGBCUBE_H)
#define RGBCUBE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void cube_init();
void cube_start();
void cube_stop();
void cube_effect_timeout(uint32_t);
void cube_set_brightness(uint8_t);
void cube_set_microphone_muted(bool muted);
void cube_next_effect();
void cube_prev_effect();
bool cube_set_effect(const char *name);
const char *cube_get_effect();
void cube_set_mode(const char *mode);
const char *cube_get_mode();
void cube_ota_begin();
void cube_ota_abort();

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // RGBCUBE_H
