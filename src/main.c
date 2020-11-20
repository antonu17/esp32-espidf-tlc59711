#include "effect_loop.h"
#include "event_loop.h"
#include "framebuffer.h"
#include "http/http.h"
#include "spiffs.h"
#include "wifi.h"
#include "stats.h"
#include "effect_list.h"

void app_main() {
    init_framebuffer();
    init_event_loop();
    init_effect_list();
    init_effects();
    init_stats();
    init_spiffs();
    init_wifi();
    init_http();
}
