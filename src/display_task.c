//
// Created by Samuel Jones on 11/26/21.
//

#include "display_task.h"
#include "display_buffer.h"
#include "display_draw_text.h"
#include "display_draw_geometry.h"
#include "epaper_hal.h"

#include <stdio.h>

static TaskHandle_t _displayHandle;


#define MIN(x, y) ((x) >= (y) ? (y) : (x))

void _Noreturn display_task(void* params) {

    EPAPER_Init();
    static int i = 150;

    while (1) {
        vTaskDelay(100 * 1000 / configTICK_RATE_HZ);

        const char *test_string = "the quick brown fox jumps over the lazy dog.";
        const char *test_string_2 = "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.";
        const char *txest_string_3 = "0123456789 :\"!?/<>[]{}()_@#$%^&*,.;:'\\";
        DISPBUF_Swap();
        DISPBUF_ClearActive();

        DISPLAY_COORD cursor = {5, 5};
        DISPBUF_DrawMultiline(cursor, test_string, BITTER_PRO_24, 400, 400, 0);
        DISPBUF_DrawMultiline(cursor, test_string_2, BITTER_PRO_24, 400, 400, DRAW_TEXT_JUSTIFY_VERT_CENTER | DRAW_TEXT_JUSTIFY_HORIZ_CENTER);
        DISPBUF_DrawMultiline(cursor, test_string_3, BITTER_PRO_24, 400, 400, DRAW_TEXT_JUSTIFY_VERT_BOTTOM | DRAW_TEXT_JUSTIFY_HORIZ_RIGHT);

        EPAPER_RenderBuffer(DISPBUF_ActiveBuffer(), DISPBUF_InactiveBuffer(), BUFFER_SIZE);
        i++;
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