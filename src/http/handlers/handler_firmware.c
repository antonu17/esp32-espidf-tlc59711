#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_wifi.h>
#include <sys/param.h>

#define TAG __FILE__
#define MAX_FILE_SIZE (1024 * 1024)  // 1 MB
#define MAX_FILE_SIZE_STR "1MB"
#define BUFSIZE 1024

static char ota_write_data[BUFSIZE + 1] = {0};

static void post_update();

esp_err_t upload_firmware_handler(httpd_req_t *req) {

    /* File cannot be larger than a limit */
    if (req->content_len > MAX_FILE_SIZE) {
        ESP_LOGE(TAG, "File too large : %d bytes", req->content_len);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                            "File size must be less than " MAX_FILE_SIZE_STR "!");
        /* Return failure to close underlying connection else the
         * incoming file content will keep the socket busy */
        return ESP_FAIL;
    }

    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;

    ESP_LOGI(TAG, "Starting firmware update");

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured->address, running->address);
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);

    update_partition = esp_ota_get_next_update_partition(NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);
    assert(update_partition != NULL);

    bool image_header_was_checked = false;

    ESP_LOGI(TAG, "Receiving firmware...");

    int received;

    /* Content length of the request gives
     * the size of the file being uploaded */
    int remaining = req->content_len;

    while (remaining > 0) {
        ESP_LOGD(TAG, "Remaining size : %d", remaining);
        /* Receive the file part by part into a buffer */
        if ((received = httpd_req_recv(req, ota_write_data, MIN(remaining, BUFSIZE))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry if timeout occurred */
                continue;
            }

            /* In case of unrecoverable error,
             * close and delete the unfinished file*/
            ESP_LOGE(TAG, "File reception failed!");

            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
            return ESP_FAIL;
        }

        if (image_header_was_checked == false) {
            esp_app_desc_t new_app_info;
            if (received > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                // check current version with downloading
                memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                ESP_LOGI(TAG, "New firmware version: %s", new_app_info.version);

                esp_app_desc_t running_app_info;
                if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
                    ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
                }

                const esp_partition_t *last_invalid_app = esp_ota_get_last_invalid_partition();
                esp_app_desc_t invalid_app_info;
                if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
                    ESP_LOGI(TAG, "Last invalid firmware version: %s", invalid_app_info.version);
                }

                // check current version with last invalid partition
                if (last_invalid_app != NULL) {
                    if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
                        ESP_LOGW(TAG, "New version is the same as invalid version.");
                        ESP_LOGW(TAG, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                        ESP_LOGW(TAG, "The firmware has been rolled back to the previous version.");
                        httpd_resp_send_err(req, HTTPD_505_VERSION_NOT_SUPPORTED, "New version is the same as invalid version.");
                        return ESP_FAIL;
                    }
                }
                if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
                    ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
                    httpd_resp_send_err(req, HTTPD_505_VERSION_NOT_SUPPORTED, "Current running version is the same as a new. We will not continue the update.");
                    return ESP_FAIL;
                }

                image_header_was_checked = true;

                err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, esp_err_to_name(err));
                    return ESP_FAIL;
                }
                ESP_LOGI(TAG, "esp_ota_begin succeeded");
            } else {
                ESP_LOGE(TAG, "received package is not fit len");
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "received package is not fit len");
                return ESP_FAIL;
            }
        }

        /* Write buffer content to file on storage */
        err = esp_ota_write(update_handle, (const void *)ota_write_data, received);
        if (err != ESP_OK) {
            /* Couldn't write everything to file!
             * Storage may be full? */
            ESP_LOGE(TAG, "Firmware write failed!");

            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to write firmware");
            return ESP_FAIL;
        }

        /* Keep track of remaining size of
         * the file left to be uploaded */
        remaining -= received;
    }

    /* Close file upon upload completion */
    ESP_LOGI(TAG, "File reception complete");
    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
        }
        ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Image validation failed");
        return ESP_FAIL;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, esp_err_to_name(err));
        return ESP_FAIL;
    }
    xTaskCreatePinnedToCore(post_update, "post_update", 1024, NULL, 2, NULL, 0);
    return ESP_OK;
}

static void post_update() {
    vTaskDelay(pdMS_TO_TICKS(3000));
    ESP_LOGI(TAG, "Prepare to restart system!");
    esp_restart();
    vTaskDelete(NULL);
}
