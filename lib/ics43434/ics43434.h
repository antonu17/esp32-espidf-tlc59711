#if !defined(ICS43434_H)
#define ICS43434_H

#include <freertos/FreeRTOS.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ICS43434_SPECTRUM_BARS 8

void ics43434_init(void);
void ics43434_set_muted(bool muted);
bool ics43434_is_muted(void);
void ics43434_set_spectrum_enabled(bool enabled);
bool ics43434_read_spectrum(float bars[ICS43434_SPECTRUM_BARS],
                            TickType_t timeout);
bool ics43434_read_bpm(float *bpm, float *confidence, TickType_t timeout);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ICS43434_H
