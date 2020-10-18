#include <dirent.h>
#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"

#include "event_loop.h"
#include "spiffs.h"
#include "mic.h"
#include "wifi.h"
#include "http/http.h"
#include "framebuffer.h"
#include "effects/effects.h"

static const char *TAG = "MAIN";

void app_main() {
    init_spiffs();
    init_event_loop();
    init_mic();
    init_wifi();
    init_http();
    init_framebuffer();
    init_effects();
}
