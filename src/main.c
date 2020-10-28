#include "effects/effects.h"
#include "event_loop.h"
#include "framebuffer.h"
#include "http/http.h"
#include "spiffs.h"
#include "wifi.h"

void app_main() {
    init_spiffs();
    init_event_loop();
    init_wifi();
    init_http();
    init_framebuffer();
    init_effects();
}
