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
#define INTERNAL_MOUNT_POINT "/int/"

typedef void* file_handle;

// FS characteristics should be known by hardware.
int FS_Mount(void);
file_handle FS_Open(const char* name, const char* mode);
int FS_Read(file_handle handle, void* buf, int len);
int FS_Write(file_handle handle, void* buf, int len);
int FS_Fseek(file_handle handle, int offset, int whence);
int FS_Remove(const char* name);
int FS_Stat(const char* path, struct stat* stat);
int FS_Close(file_handle handle);
int FS_Rename(const char* old, const char* new);
int FS_Feof(file_handle handle);

#endif //EPAPER_DISPLAY_FILESYSTEM_HAL_H
