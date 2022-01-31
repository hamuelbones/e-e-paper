//
// Created by Samuel Jones on 1/22/22.
//

#include <stdbool.h>
#include "filesystem_hal.h"

#ifndef EPAPER_DISPLAY_FILE_UTILS_H
#define EPAPER_DISPLAY_FILE_UTILS_H

bool file_copy(const char* to, const char* from);
bool file_exists(const char* path);

bool file_load_uuid(const char* path, char uuid[37]);
size_t toml_fs_read(void* ptr, size_t size, size_t nitems, void* stream);
void file_print(const char* path);

#endif //EPAPER_DISPLAY_FILE_UTILS_H
