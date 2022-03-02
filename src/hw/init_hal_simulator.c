//
// Created by Samuel Jones on 3/1/22.
//

#include <stdlib.h>

void app_hal_reboot(void) {
    exit(0);
}

const char* app_hal_version(void) {
    return "0.0.0";
}

const char* app_hal_name(void) {
    return "epaper-display-simulator";
}