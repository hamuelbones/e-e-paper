//
// Created by Samuel Jones on 1/22/22.
//

#include "toml_resources.h"
#include "filesystem_hal.h"
#include <string.h>
#include <FreeRTOS.h>

static TOML_RESOURCE_CTX _tomlResources;

bool toml_resource_load(const char* file_name, const char* resource_name) {

    if (_tomlResources.count >= MAX_TOML_RESOURCES) {
        return false;
    }

    file_handle f = FS_Open(file_name, "r");
    if (!f) {
        return false;
    }

    char toml_error_msg[100];
    toml_table_t *table = toml_parse_file(f, toml_error_msg, 100);
    if (!table) {
        return false;
    }

    size_t res_name_len = strlen(resource_name);

    size_t new_id = _tomlResources.count;
    TOML_RESOURCE *new_res = &_tomlResources.resource[new_id];

    new_res->name = pvPortMalloc(res_name_len+1);
    strncpy(new_res->name, resource_name, res_name_len+1);
    new_res->table = table;

    _tomlResources.count++;
    return true;
}

toml_table_t* toml_resource_get(const char* name) {

    for (int i=0; i<_tomlResources.count; i++) {
        if (0 == strcmp(_tomlResources.resource[i].name, name)) {
            return _tomlResources.resource[i].table;
        }
    }
    return NULL;

}

bool toml_resource_unload_all() {
    for (int i=0; i<_tomlResources.count; i++) {
        vPortFree(_tomlResources.resource[i].name);
        toml_free(_tomlResources.resource[i].table);
    }
    _tomlResources.count = 0;
    return true;
}