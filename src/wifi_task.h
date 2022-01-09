//
// Created by Samuel Jones on 11/26/21.
//

#ifndef EPAPER_DISPLAY_WIFI_TASK_H
#define EPAPER_DISPLAY_WIFI_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef enum {
    WIFI_CONNECT,
    WIFI_DISCONNECT,
    WIFI_GET,
} WIFI_REQUEST_TYPE;

typedef struct {
    char ssid[30];
    char password[60];
} WIFI_CONNECT_PARAMS;

typedef struct {

    union {

    };
} WIFI_HTTP_PARAMS;

void wifi_task_start(void);
TaskHandle_t wifi_task_handle(void);

void WIFI_Request();

#endif //EPAPER_DISPLAY_WIFI_TASK_H
