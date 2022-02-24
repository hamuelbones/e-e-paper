//
// Created by Samuel Jones on 1/22/22.
//

#include "toml_resources.h"
#include "filesystem_hal.h"
#include "toml.h"
#include "freertos/FreeRTOS.h"

void * toml_load(const char* file_name) {

    file_handle f = fs_open(file_name, "r");
    if (!f) {
        return NULL;
    }

    char toml_error_msg[100];
    toml_table_t *table = toml_parse_file(f, toml_error_msg, 100);
    fs_close(f);

    if (!table) {
        return NULL;
    }

    return (void*) table;
}

void toml_unload(void* table) {
    toml_free((toml_table_t *)table);
}

