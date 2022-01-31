//
// Created by Samuel Jones on 1/7/22.
//

#include <filesystem_hal.h>

//
// Created by Samuel Jones on 1/7/22.
//

#include <stdio.h>
#include <sys/stat.h>
#include "driver/sdspi_host.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_spiffs.h"

typedef void* file_handle;

#define GPIO_SD_CS (GPIO_NUM_8)
static const char *TAG = "example";

// FS characteristics should be known by hardware.
int FS_Mount(void) {

    esp_err_t ret;

    esp_vfs_spiffs_conf_t spiffs_conf = {
        .base_path = INTERNAL_MOUNT_FOLDER,
        .format_if_mount_failed = true,
        .max_files = 2,
        .partition_label = "fs",
    };
    ret = esp_vfs_spiffs_register(&spiffs_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount internal flash partition, %08x", ret);
        return -1;
    }
    ESP_LOGI(TAG, "Internal filesystem mounted");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = GPIO_SD_CS;
    slot_config.host_id = host.slot;

    // Options for mounting the filesystem.
    esp_vfs_fat_sdmmc_mount_config_t sd_mount_config = {
            .format_if_mount_failed = false,
            .max_files = 2,
            .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t *card;
    ESP_LOGI(TAG, "Mounting filesystem");
    // TODO recommended to do more proper SD card probing + partition mounting manually
    ret = esp_vfs_fat_sdspi_mount(SD_MOUNT_FOLDER, &host, &slot_config, &sd_mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                          "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return -1;
    }
    ESP_LOGI(TAG, "Filesystem mounted");
    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);


    return 0;
}
void FS_Unmount(void) {
    // Not implemented
}

file_handle FS_Open(const char* name, const char* mode) {
    return fopen(name, mode);
}
int FS_Read(file_handle handle, void* buf, int len) {
    return (int) fread(buf, 1, len, handle);
}
int FS_Write(file_handle handle, void* buf, int len) {
    return (int) fwrite(buf, 1, len, handle);
}
int FS_Fseek(file_handle handle, int offset, int whence) {
    return fseek(handle, offset, whence);
}
int FS_Remove(const char* name) {
    return remove(name);
}
int FS_Stat(const char* path, struct stat* fstat) {
    return (int) stat(path, fstat);
}
int FS_Close(file_handle handle) {
    return fclose(handle);
}
int FS_Rename(const char* old, const char* new) {
    return rename(old, new);
}
int FS_Feof(file_handle handle) {
    return feof(handle);
}