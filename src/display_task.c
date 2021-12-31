//
// Created by Samuel Jones on 11/26/21.
//

#include "display_task.h"
#include "display_buffer.h"
#include "display_draw_text.h"
#include "display_draw_geometry.h"
#include "epaper_hal.h"

static TaskHandle_t _displayHandle;


void _Noreturn display_task(void* params) {

    EPAPER_Init();
    static int i = 0;

    while (1) {
        vTaskDelay(100 * 1000 / configTICK_RATE_HZ);

        char test_string[100];
        snprintf(test_string, 100, "Hello font-rendering world! %u %u Happy wedding Brandon and Kenzie!", i, i);

        DISPBUF_Swap();
        DISPBUF_ClearActive();

        DISPLAY_COORD cursor = {5, 100};
        DISPBUF_DrawLabel(cursor, test_string, HELVETICA_14);

        cursor.y = 5;
        DISPBUF_DrawLabel(cursor, "TEST", HELVETICA_14);

        cursor.x = 400;
        DISPBUF_DrawLabel(cursor, "TESTX", HELVETICA_14);

        cursor.y = 400;
        DISPBUF_DrawLabel(cursor, "TESTXY", HELVETICA_14);

        cursor.x = 5;
        DISPBUF_DrawLabel(cursor, "TESTY", HELVETICA_14);

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