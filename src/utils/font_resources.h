//
// Created by Samuel Jones on 2/1/22.
//
// font_resources - loading and unloading font files into font tables

#include <stdbool.h>
#include "toml.h"
#include "fonts.h"

#ifndef EPAPER_DISPLAY_FONT_RESOURCES_H
#define EPAPER_DISPLAY_FONT_RESOURCES_H

// TODO Future enhancement: make this dynamic and non-singleton
// TODO Merge with font resources code
#define MAX_FONT_RESOURCES (5)

typedef struct {
    char* name;
    FONT_TABLE* font;
} FONT_RESOURCE;

typedef struct {
    FONT_RESOURCE resource[MAX_FONT_RESOURCES];
    unsigned int count;
} FONT_RESOURCE_CTX;

bool font_resource_load(const char* file_name, const char* resource_name);

FONT_TABLE* font_resource_get(const char* name);
bool font_resource_unload_all();

#endif //EPAPER_DISPLAY_FONT_RESOURCES_H
