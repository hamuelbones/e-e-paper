//
// Created by Samuel Jones on 2/23/22.
//


#include "toml_resources.h"
#include "font_resources.h"
#include "resources.h"
#include "filesystem_hal.h"

#include "freertos/FreeRTOS.h"
#include <string.h>

typedef struct {
    void* (*load_file)(const char* filename);
    void  (*unload_data)(void* context);
    void  (*new_frame)(void* context);
    void* (*get_element)(void* context, const char* key);
} RESOURCE_LOAD_INTERFACE;

typedef struct {
    RESOURCE_TYPE type;
    char * name;
    void * data;
} RESOURCE;

typedef struct {
    RESOURCE resource[MAX_RESOURCES];
    unsigned int count;
} RESOURCE_CTX;

static RESOURCE_CTX resources;

static const RESOURCE_LOAD_INTERFACE interfaces[RESOURCE_MAX] = {
        [RESOURCE_TOML] = {toml_load, toml_unload, toml_new_frame, toml_get_element},
        [RESOURCE_FONT] = {font_load, font_unload}
};

bool resource_load(const char* file_name, const char *resource_name, RESOURCE_TYPE type) {
    void* loaded_resource = interfaces[type].load_file(file_name);
    if (!loaded_resource) {
        return false;
    }

    size_t new_id = resources.count;
    RESOURCE *new_resource = &resources.resource[new_id];

    size_t resource_name_len = strlen(resource_name);
    new_resource->name = pvPortMalloc(resource_name_len+1);
    strcpy(new_resource->name, resource_name);
    new_resource->data = loaded_resource;
    new_resource->type = type;
    resources.count++;

    return true;
}

void* resource_get(const char* name) {
    for (int i=0; i<resources.count; i++) {
        if (0 == strcmp(resources.resource[i].name, name)) {
            return resources.resource[i].data;
        }
    }
    return NULL;
}

bool resource_unload(const char* name) {
    for (int i=0; i<resources.count; i++) {
        if (0 == strcmp(resources.resource[i].name, name)) {
            RESOURCE* to_unload = &resources.resource[i];
            vPortFree(to_unload->name);
            interfaces[to_unload->type].unload_data(to_unload->data);
            for (int j=i; j<resources.count-1; j++) {
                resources.resource[j] = resources.resource[j+1];
            }
            resources.count--;
            return true;
        }
    }
    return false;
}

bool resource_unload_all(void) {
    while (resources.count) {
        resource_unload(resources.resource[0].name);
    }
    return true;
}

void resource_new_frame(void) {
    for (int i=0; i<resources.count; i++) {
        RESOURCE_TYPE type = resources.resource[i].type;
        if (interfaces[type].new_frame) {
            interfaces[type].new_frame(resources.resource[i].data);
        }
    }
}

void* resource_get_element(const char* key) {

    char resource_name[30];
    char* divider = strchr(key, ':');
    if (!divider) {
        return NULL;
    }

    size_t name_len = divider-key;
    if (name_len > 29) {
        name_len = 29;
    }
    memcpy(resource_name, key, name_len);
    resource_name[name_len] = '\0';

    for (int i=0; i<resources.count; i++) {
        if (0 == strcmp(resources.resource[i].name, resource_name)) {
            // Found the resource
            RESOURCE_TYPE type = resources.resource[i].type;
            if (!interfaces[type].get_element) {
                return NULL;
            }
            return interfaces[type].get_element(resources.resource[i].data, divider+1);
        }
    }
    return NULL;
}