#include "nvs.h"

#include <nvs_flash.h>
#include <stdio.h>
#include <stdlib.h>

void write_nvs_str(const char* ns, const char* key, const char* data) {
    esp_err_t err;
    nvs_handle_t my_handle;

    err = nvs_open(ns, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        return;
    }

    err = nvs_set_str(my_handle, key, data);

    nvs_close(my_handle);
}

char* read_nvs_str(const char* ns, const char* key) {
    esp_err_t err;
    nvs_handle_t my_handle;
    char* result = "";

    err = nvs_open(ns, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        return result;
    }

    // Read the size of memory space required for blob
    size_t required_size = 0;  // value will default to 0, if not set yet in NVS
    err = nvs_get_str(my_handle, key, NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return result;
    printf("Reading %d bytes\n", required_size);

    // Read previously saved blob if available
    if (required_size > 0) {
        char* read_data = malloc(required_size + sizeof(char));
        err = nvs_get_str(my_handle, key, read_data, &required_size);
        if (err != ESP_OK) {
            printf("Error (%s) reading key (%s)!\n", esp_err_to_name(err), key);
            free(read_data);
            return result;
        }
        result = read_data;
    }

    nvs_close(my_handle);
    return result;
}

void write_nvs_int(const char* ns, const char* key, const int data) {
    esp_err_t err;
    nvs_handle_t my_handle;

    err = nvs_open(ns, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        return;
    }

    err = nvs_set_i32(my_handle, key, data);

    nvs_close(my_handle);
}

int read_nvs_int(const char* ns, const char* key) {
    esp_err_t err;
    nvs_handle_t my_handle;
    int result = 0;

    err = nvs_open(ns, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        return result;
    }

    err = nvs_get_i32(my_handle, key, &result);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return result;

    nvs_close(my_handle);
    return result;
}

void init_storage() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}
