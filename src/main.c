#include "wifi.h"
#include "http.h"
#include "framebuffer.h"
#include "effects/effects.h"

void app_main() {
    init_wifi();
    init_http();
    init_framebuffer();
    init_effects();
}
