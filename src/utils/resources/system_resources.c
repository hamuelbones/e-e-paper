//
// Created by Samuel Jones on 3/1/22.
//

#include "system_resources.h"
#include "init_hal.h"
#include <string.h>
#include "freertos/FreeRTOS.h"

void* system_load(const char* file_name) {
    return (void*)1;
}
void system_unload(void* table) {

}
void* system_get_element(void* table, const char *key) {
    if (strcmp(key, "version") == 0) {
        const char *ver = app_hal_version();
        size_t len = strlen(ver);
        char* ver_copy = pvPortMalloc(len+1);
        strcpy(ver_copy, ver);
        return ver_copy;
    } else if (strcmp(key, "name") == 0) {
        const char *name = app_hal_name();
        size_t len = strlen(name);
        char* name_copy = pvPortMalloc(len+1);
        strcpy(name_copy, name);
        return name_copy;
    }
    return NULL;
}
