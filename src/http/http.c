#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <mdns.h>

#include "../nvs.h"
#include "sdkconfig.h"
#include "spiffs.h"

static const char *TAG = __FILE__;

esp_err_t start_rest_server(const char *base_path);

static void initialise_mdns(void) {
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set("rgb-cube"));
    ESP_ERROR_CHECK(mdns_instance_name_set("RGB LED Cube (awesome)"));

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"path", "/"}};

    ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData,
                                     sizeof serviceTxtData / sizeof serviceTxtData[0]));
    ESP_LOGI(TAG, "mDNS initialized");
}

void init_http(void) {
    init_nvs();
    ESP_ERROR_CHECK(esp_netif_init());

    initialise_mdns();

    ESP_ERROR_CHECK(start_rest_server("/spiffs"));
    list_dir("/spiffs");
}
