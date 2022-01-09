//
// Created by Samuel Jones on 1/7/22.
//

#include <filesystem_hal.h>

const char* filesystem_base = "/Users/ham/src/e-e-paper/src/hw/simulator_filesystem/";

static void _full_path(const char* name, char* fullName, int len) {
    snprintf(fullName, len, "%s%s", filesystem_base, name);
}

int FS_Mount(void) {
    return 0;
}

void FS_Unmount(void) {

}

file_handle FS_Open(const char* name, const char* mode) {
    char fullName[200];
    _full_path(name, fullName, 200);
    return fopen(fullName, mode);
}

int FS_Read(file_handle handle, void* buf, int len) {
    return (int) fread(buf, 1, len, handle);
}

int FS_Write(file_handle handle, void* buf, int len) {
    return (int) fwrite(buf, 1, len, handle);
}

int FS_Fseek(file_handle handle, int offset, int whence) {
    return (int) fseek(handle, offset, whence);
}

int FS_Remove(const char* name) {
    char fullName[200];
    _full_path(name, fullName, 200);
    return (int) remove(fullName);
}

int FS_Stat(const char* name, struct stat* fstat) {
    char fullName[200];
    _full_path(name, fullName, 200);
    return (int) stat(fullName, fstat);
}

int FS_Close(file_handle handle) {
    return fclose((FILE*)handle);
}

int FS_Rename(const char* old, const char* new) {
    char fullOldName[200];
    _full_path(old, fullOldName, 200);
    char fullNewName[200];
    _full_path(new, fullNewName, 200);
    return rename(fullOldName, fullNewName);
}

int FS_NumFiles(void) {
    printf("Not Implemented!");
    return -1;
}

int FS_NthFile(int n) {
    printf("Not Implemented!");
    return -1;
}

int FS_Feof(file_handle handle) {
    return feof(handle);
}