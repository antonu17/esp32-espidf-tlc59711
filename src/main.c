#include "effect_loop.h"
#include "event_loop.h"
#include "framebuffer.h"
#include "http/http.h"
#include "spiffs.h"
#include "wifi.h"

void app_main() {
    init_framebuffer();
    init_event_loop();
    init_effects();
    init_spiffs();
    init_wifi();
    init_http();
}
