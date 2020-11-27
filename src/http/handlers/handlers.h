
#if !defined(HTTP_HANDLERS_H)
#define HTTP_HANDLERS_H

#include <esp_http_server.h>
#include <esp_vfs.h>

#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do {                                                                               \
        if (!(a)) {                                                                    \
            ESP_LOGE(REST_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

esp_err_t cube_mode_get_handler(httpd_req_t *req);
esp_err_t effect_list_get_handler(httpd_req_t *req);
esp_err_t cube_mode_switch_handler(httpd_req_t *req);
esp_err_t upload_firmware_handler(httpd_req_t *req);

#endif  // HTTP_HANDLERS_H
