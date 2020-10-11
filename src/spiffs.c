
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"

#include "spiffs.h"

#define TAG "SPIFFS"
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)

int fileLength( FILE *f ){
    int pos;
    int end;

    // seeks the beginning of the file to the end and counts
    // it and returns into variable end
    pos = ftell(f);
    fseek (f, 0, SEEK_END);
    end = ftell(f);
    fseek (f, pos, SEEK_SET);

    return end;
}

void list_dir(const char *name) {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char path[FILE_PATH_MAX] = {0};

    ESP_LOGI(TAG, "List files from: %s", name);

    if (!(dir = opendir(name))) {
        ESP_LOGE(TAG, "Failed to open dir: %s", name);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        snprintf(path, sizeof(path), "%s/", name);
        strcat(path, entry->d_name);
        if (stat(path, &st) == -1) {
            ESP_LOGE(TAG, "Failed to stat: %s", path);
        }
        printf("%s (%ld)\n", path, st.st_size);
    }

    closedir(dir);
}

void init_spiffs() {

    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = BASE_PATH,
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

}
