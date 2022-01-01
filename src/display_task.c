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

        char test_string[100];
        snprintf(test_string, 100, "Hello font-rendering world! %u %u Happy wedding Brandon and Kenzie!", i, i);

        DISPBUF_Swap();
        DISPBUF_ClearActive();

        DISPLAY_COORD cursor = {5, 100};
        DISPBUF_DrawLabel(cursor, test_string, HELVETICA_14, 0);

        cursor.x = 420;
        cursor.y = 5;
        const char *lipsum = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
        //const char *lipsum = "Hello! asdfasdfasdf asdfasdfasdf asdf";
        DISPBUF_DrawMultiline(cursor, lipsum, HELVETICA_14, 300-MIN(i, 290), 400, DRAW_TEXT_JUSTIFY_CENTER);

        DISPBUF_DrawHorizontalLine(200, 200, 400);
        DISPBUF_DrawVerticalLine(200, 200, 400);

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