/* HTTP Restful API Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "sdmmc_cmd.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mdns.h"
#include "lwip/apps/netbiosns.h"

#define MDNS_INSTANCE "esp home web server"

static const char *TAG = "HTTP";

esp_err_t start_rest_server(const char *base_path);
esp_err_t start_file_server(const char *base_path);

static void initialise_mdns(void)
{
    mdns_init();
    mdns_hostname_set("esp-home");
    mdns_instance_name_set(MDNS_INSTANCE);

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"path", "/"}
    };

    ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));
}

void init_http(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());

    initialise_mdns();
    netbiosns_init();
    netbiosns_set_name("esp-home");

    ESP_ERROR_CHECK(start_file_server("/spiffs"));
    // ESP_ERROR_CHECK(start_rest_server("/www"));
}
