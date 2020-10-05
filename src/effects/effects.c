#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <soc/soc.h>

#include "effects.h"

void effect_loop() {
    cube_color_wheel();
}

void init_effects() {
    // Effect loop function is running as a low priority task
    xTaskCreatePinnedToCore(effect_loop, "effect_loop", 4096, NULL, tskIDLE_PRIORITY, NULL, PRO_CPU_NUM);
}
