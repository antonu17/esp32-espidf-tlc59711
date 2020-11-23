#include "persistence.h"

#include <esp_log.h>

#include "../nvs.h"

const char* NS = "coob";
const char* MODE = "mode";
const char* EFFECT = "effect";

void save_mode(coob_mode_t mode) {
    write_nvs_int(NS, MODE, mode);
    ESP_LOGD(__FILE__, "saved mode: %d", mode);
}

coob_mode_t load_mode() {
    int mode = read_nvs_int(NS, MODE);
    ESP_LOGD(__FILE__, "loaded mode: %d", mode);
    return mode;
}

void save_effect(const char* effect) {
    write_nvs_str(NS, EFFECT, effect);
    ESP_LOGD(__FILE__, "saved effect: %s", effect);
}

char* load_effect() {
    char* effect = read_nvs_str(NS, EFFECT);
    ESP_LOGD(__FILE__, "loaded effect: %s", effect);
    return effect;
}
