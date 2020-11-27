#include "coob.h"
#include "event_loop.h"
#include "firmware_update.h"
#include "framebuffer.h"
#include "http.h"
#include "spiffs.h"
#include "stats.h"
#include "wifi.h"

void app_main() {
    init_framebuffer();
    init_ota();
    init_event_loop();
    init_coob();
    init_stats();
    init_spiffs();
    init_wifi();
    init_http();
}
