//
// Created by Samuel Jones on 11/26/21.
//

#ifndef EPAPER_DISPLAY_WIFI_TASK_H
#define EPAPER_DISPLAY_WIFI_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void wifi_task_start(void);
TaskHandle_t wifi_task_handle(void);

#endif //EPAPER_DISPLAY_WIFI_TASK_H
