//
// Created by Samuel Jones on 2/23/22.
//

#ifndef EPAPER_DISPLAY_RESOURCES_H
#define EPAPER_DISPLAY_RESOURCES_H

#include <stdbool.h>
#include "toml_resources.h"
#include "font_resources.h"
#include "system_resources.h"

typedef enum {
    RESOURCE_SYSTEM,
    RESOURCE_TOML,
    RESOURCE_FONT,
    RESOURCE_CLOCK,
    RESOURCE_MAX,
} RESOURCE_TYPE;

// TODO Future enhancement: make this dynamic and non-singleton
#define MAX_RESOURCES (10)

bool resource_load(const char* file_name, const char *resource_name, RESOURCE_TYPE type);
void* resource_get(const char* name);

bool resource_unload(const char* name);
bool resource_unload_all(void);
void resource_new_frame(void);
void* resource_get_element(const char* key);

char *resource_make_substitutions(const char *input);

#endif //EPAPER_DISPLAY_RESOURCES_H
