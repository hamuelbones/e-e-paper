//
// Created by Samuel Jones on 1/8/22.
//

#include "toml.h"

#ifndef EPAPER_APPS_H
#define EPAPER_APPS_H

typedef struct {
    const char* name;
    int refresh_rate_ms;
    void* (*app_init)(toml_table_t *startup_config, toml_table_t* device_config);
    void (*app_process)(void* context, uint8_t *message, size_t length);
    void (*app_deinit)(void* context);
} APP_INTERFACE;

#endif /* EPAPER_APPS_H */
