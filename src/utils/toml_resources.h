//
// Created by Samuel Jones on 1/22/22.
//

#include <stdbool.h>
#include "toml.h"

#ifndef EPAPER_DISPLAY_TOML_RESOURCES_H
#define EPAPER_DISPLAY_TOML_RESOURCES_H

// Future enhancement: make this dynamic and non-singleton
#define MAX_TOML_RESOURCES (10)

typedef struct {
    char* name;
    toml_table_t *table;
} TOML_RESOURCE;

typedef struct {
    TOML_RESOURCE resource[MAX_TOML_RESOURCES];
    unsigned int count;
} TOML_RESOURCE_CTX;

bool toml_resource_load(const char* file_name, const char* resource_name);

toml_table_t* toml_resource_get(const char* name);
bool toml_resource_unload_all();

#endif //EPAPER_DISPLAY_TOML_RESOURCES_H
