//
// Created by Samuel Jones on 1/7/22.
//

#include <stdio.h>
#include <sys/stat.h>

#ifndef EPAPER_DISPLAY_FILESYSTEM_HAL_H
#define EPAPER_DISPLAY_FILESYSTEM_HAL_H


#define STARTUP_FILENAME "startup.toml"
#define APP_CONFIG_FILENAME "config.toml"
#define REQUEST_TEMPORARY_FILENAME "temp.bin"

#define SD_MOUNT_POINT "/sd/"
#define SD_MOUNT_FOLDER "/sd"
#define INTERNAL_MOUNT_POINT "/int/"
#define INTERNAL_MOUNT_FOLDER "/int"

typedef void* file_handle;

// FS characteristics should be known by hardware.
int fs_mount(void);
file_handle fs_open(const char* name, const char* mode);
int fs_read(file_handle handle, void* buf, int len);
int fs_write(file_handle handle, void* buf, int len);
int fs_fseek(file_handle handle, int offset, int whence);
int fs_remove(const char* name);
int fs_stat(const char* path, struct stat* stat);
int fs_close(file_handle handle);
int fs_rename(const char* old, const char* new);
int fs_feof(file_handle handle);

#endif //EPAPER_DISPLAY_FILESYSTEM_HAL_H
