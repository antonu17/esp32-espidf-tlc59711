#include <cJSON.h>
#include <esp_http_server.h>
#include <esp_log.h>

#include "coob.h"
#include "effect_list.h"
#include "handlers.h"

#define TAG __FILE__

esp_err_t cube_mode_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/json");
    coob_t coob = coob_get_instance();
    int mode = coob_get_mode(coob);
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "mode", mode);

    const char *response = cJSON_Print(root);
    httpd_resp_sendstr(req, response);
    free((void *)response);
    cJSON_Delete(root);

    return ESP_OK;
}

esp_err_t effect_list_get_handler(httpd_req_t *req) {
    int effect_count;
    cJSON *root, *effects;
    httpd_resp_set_type(req, "application/json");

    effect_count = effect_list_length(effect_list);
    root = cJSON_CreateObject();
    effects = cJSON_AddArrayToObject(root, "effects");
    cJSON_AddNumberToObject(root, "effect_count", effect_count);

    for (int i = 0; i < effect_count; i++) {
        cJSON_AddItemToArray(effects, cJSON_CreateString(effect_list_get_by_idx(effect_list, i)->name));
    }

    const char *response = cJSON_Print(root);
    httpd_resp_sendstr(req, response);
    free((void *)response);
    cJSON_Delete(root);

    return ESP_OK;
}

esp_err_t cube_mode_switch_handler(httpd_req_t *req) {
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    coob_t coob = NULL;

    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    cJSON *root = cJSON_Parse(buf);
    int mode = -1, effect = -1;
    if (cJSON_GetObjectItem(root, "mode")) {
        mode = cJSON_GetObjectItem(root, "mode")->valueint;
    }
    if (cJSON_GetObjectItem(root, "effect")) {
        effect = cJSON_GetObjectItem(root, "effect")->valueint;
    }
    ESP_LOGI(TAG, "Mode: %d", mode);
    ESP_LOGI(TAG, "Effect: %d", effect);

    coob = coob_get_instance();
    switch (mode) {
        case 0:
            coob_mode_loop(coob);
            break;
        case 1:
            coob_mode_solo(coob);
            break;
        default:
            ESP_LOGI(TAG, "Wrong mode provided: %d", mode);
            break;
    }
    if (effect >= 0) {
        ESP_LOGI(TAG, "Switch effect: %d", effect);
        coob_switch_effect(coob, effect);
    }

    cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post control value successfully");
    return ESP_OK;
}
