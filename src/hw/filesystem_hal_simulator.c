//
// Created by Samuel Jones on 1/7/22.
//

#include <stdlib.h>
#include "filesystem_hal.h"

// TODO make this relative or configurable
const char* filesystem_base_default = "./";

static void _full_path(const char* name, char* fullName, int len) {

    const char* filesystem_base = getenv("SIMULATOR_FS_BASE");
    if (!filesystem_base) {
        filesystem_base = filesystem_base_default;
    }

    snprintf(fullName, len, "%s%s", filesystem_base, name);
}

int fs_mount_internal(void) {

    char fullPath[200];
    _full_path(INTERNAL_MOUNT_POINT, fullPath, 200);
    mkdir(fullPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    return 0;
}

int fs_mount_external(void) {

    char fullPath[200];
    _full_path( SD_MOUNT_POINT, fullPath, 200);
    mkdir(fullPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    return 0;
}

void fs_unmount_internal(void) {

}

void fs_unmount_external(void) {

}

bool fs_internal_mounted(void) {
    return true;
}

bool fs_external_mounted(void) {
    return true;
}

file_handle fs_open(const char* name, const char* mode) {
    char fullName[200];
    _full_path(name, fullName, 200);
    return fopen(fullName, mode);
}

int fs_read(file_handle handle, void* buf, int len) {
    return (int) fread(buf, 1, len, handle);
}

int fs_write(file_handle handle, void* buf, int len) {
    return (int) fwrite(buf, 1, len, handle);
}

int fs_fseek(file_handle handle, int offset, int whence) {
    return (int) fseek(handle, offset, whence);
}

int fs_remove(const char* name) {
    char fullName[200];
    _full_path(name, fullName, 200);
    return (int) remove(fullName);
}

int fs_stat(const char* name, struct stat* fstat) {
    char fullName[200];
    _full_path(name, fullName, 200);
    return (int) stat(fullName, fstat);
}

int fs_close(file_handle handle) {
    return fclose((FILE*)handle);
}

int fs_rename(const char* old, const char* new) {
    char fullOldName[200];
    _full_path(old, fullOldName, 200);
    char fullNewName[200];
    _full_path(new, fullNewName, 200);
    return rename(fullOldName, fullNewName);
}

int fs_NumFiles(void) {
    printf("Not Implemented!");
    return -1;
}

int fs_NthFile(int n) {
    printf("Not Implemented!");
    return -1;
}

int fs_feof(file_handle handle) {
    return feof(handle);
}