//
// Created by Samuel Jones on 11/26/21.
//

#include "display_task.h"
#include "display_buffer.h"
#include "display_draw_text.h"
#include "display_draw_geometry.h"
#include "epaper_hal.h"

#include "filesystem_hal.h"
#include "toml.h"

#include <stdio.h>

static TaskHandle_t _displayHandle;


void _Noreturn display_task(void* params) {

    epaper_init();

    while (1) {
        vTaskDelay(100 * 1000 / configTICK_RATE_HZ);

    }

}

void display_task_start(void) {

    xTaskCreate(display_task,
                "DISP",
                4096,
                NULL,
                5,
                &_displayHandle);
}

TaskHandle_t display_task_handle(void) {
    return _displayHandle;
}