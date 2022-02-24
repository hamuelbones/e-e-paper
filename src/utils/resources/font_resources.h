//
// Created by Samuel Jones on 2/1/22.
//
// font_resources - loading and unloading font files into font tables

#include <stdbool.h>

#ifndef EPAPER_DISPLAY_FONT_RESOURCES_H
#define EPAPER_DISPLAY_FONT_RESOURCES_H

void* font_load(const char* file_name);
void font_unload(void* table);

#endif //EPAPER_DISPLAY_FONT_RESOURCES_H
