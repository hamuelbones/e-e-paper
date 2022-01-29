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

typedef void* file_handle;

static void _full_path(const char* name, char* fullName, int len) {
    snprintf(fullName, len, "%s/%s", SD_MOUNT_POINT, name);
}

#define GPIO_SD_CS (GPIO_NUM_8)
static const char *TAG = "example";

// FS characteristics should be known by hardware.
int FS_Mount(void) {

    esp_err_t ret;

    esp_vfs_fat_mount_config_t internal_mount_config = {
            .max_files = 2,
            .format_if_mount_failed = true,
            .allocation_unit_size = 16*1024,
    };
    wl_handle_t fat_handle;
    ret = esp_vfs_fat_spiflash_mount(INTERNAL_MOUNT_POINT, "fs", &internal_mount_config, &fat_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount internal flash partition.");
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
    ret = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slot_config, &sd_mount_config, &card);

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
    char fullName[40];
    _full_path(name, fullName, 40);
    printf("%s\n", fullName);
    return fopen(fullName, mode);
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
    char fullName[40];
    _full_path(name, fullName, 40);
    return remove(fullName);
}
int FS_Stat(const char* path, struct stat* fstat) {
    char fullName[40];
    _full_path(path, fullName, 40);
    return (int) stat(fullName, fstat);
}
int FS_Close(file_handle handle) {
    return fclose(handle);
}
int FS_Rename(const char* old, const char* new) {
    char full_old_name[40];
    _full_path(old, full_old_name, 40);
    char full_new_name[40];
    _full_path(new, full_new_name, 40);
    return rename(old, new);
}
int FS_Feof(file_handle handle) {
    return feof(handle);
}

int FS_NumFiles() {
    return 0;
}
int FS_NthFile(int n) {
    return 0;
}
