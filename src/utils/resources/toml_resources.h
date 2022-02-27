//
// Created by Samuel Jones on 1/22/22.
//
// toml_resources - loading and unloading toml files into RAM

#ifndef EPAPER_DISPLAY_TOML_RESOURCES_H
#define EPAPER_DISPLAY_TOML_RESOURCES_H

#include "toml.h"

void* toml_load(const char* file_name);
void toml_unload(void* table);
void toml_new_frame(void* table);
void* toml_get_element(void* table, const char *key);

typedef struct {
    char* name;
    char* value;
} TOML_RESOURCE_VARIABLE;

typedef struct {
    TOML_RESOURCE_VARIABLE *variables;
    size_t variable_count;
    toml_table_t *document;
    int frame_count;
} TOML_RESOURCE_CONTEXT;

#endif //EPAPER_DISPLAY_TOML_RESOURCES_H
