//
// Created by Samuel Jones on 1/22/22.
//
// file_utils - helper functionality for working with files.

#include <stdbool.h>
#include "filesystem_hal.h"

#ifndef EPAPER_DISPLAY_FILE_UTILS_H
#define EPAPER_DISPLAY_FILE_UTILS_H

bool file_copy(const char* to, const char* from);

bool file_exists(const char* path);

// reads a TOML file with a device UUID inside. returns true on success and
// puts the result in uuid.
bool file_load_uuid(const char* path, char uuid[37]);

// helper function for the TOML library that reads from a file.
size_t toml_fs_read(void* ptr, size_t size, size_t nitems, void* stream);

// Primarily a testing/debugging function that prints out file contents.
void file_print(const char* path);

#endif //EPAPER_DISPLAY_FILE_UTILS_H
