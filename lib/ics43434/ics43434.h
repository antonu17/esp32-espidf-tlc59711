#if !defined(ICS43434_H)
#define ICS43434_H

#include <freertos/FreeRTOS.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ICS43434_SPECTRUM_BARS 8

void ics43434_init(void);
bool ics43434_read_spectrum(float bars[ICS43434_SPECTRUM_BARS],
                            TickType_t timeout);
float ics43434_level(void);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ICS43434_H
