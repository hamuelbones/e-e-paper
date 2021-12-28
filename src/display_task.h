//
// Created by Samuel Jones on 11/26/21.
//

#ifndef EPAPER_DISPLAY_DISPLAY_TASK_H
#define EPAPER_DISPLAY_DISPLAY_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void display_task_start(void);
TaskHandle_t display_task_handle(void);


#endif //EPAPER_DISPLAY_DISPLAY_TASK_H
