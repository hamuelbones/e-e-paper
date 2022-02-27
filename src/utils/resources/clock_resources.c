//
// Created by Samuel Jones on 2/26/22.
//

#include "clock_resources.h"
#include "time_hal.h"
#include "freertos/FreeRTOS.h"

void* clock_load(const char* dummy) {
    return (void*)1;
}

void clock_unload(void* dummy) {

}

void* clock_get_element(void* dummy, const char *key) {
    char* buf = pvPortMalloc(60);
    time_t rawtime = time(NULL);
    struct tm *ptm = localtime(&rawtime);
    strftime(buf, 60, key, ptm);
    return buf;
}