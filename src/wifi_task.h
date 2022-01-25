//
// Created by Samuel Jones on 11/26/21.
//

#ifndef EPAPER_DISPLAY_WIFI_TASK_H
#define EPAPER_DISPLAY_WIFI_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "message_buffer.h"
#include <stdbool.h>

typedef enum {
    WIFI_CONNECT,
    WIFI_DISCONNECT,
    WIFI_GET,
    WIFI_SYNC_TIME,
} WIFI_REQUEST_TYPE;

typedef struct {
    char * host;
    char * subdirectory;
    char ** headers;
    size_t header_count;
    char * headers_filename;
    char * response_filename;
} WIFI_GET_ARGS;

typedef struct {
    int status;
} WIFI_GET_RESPONSE;

typedef struct {
    char * url;
} WIFI_SYNC_TIME_ARGS;

typedef struct {
    unsigned int unix_time;
} WIFI_SYNC_TIME_RESPONSE;

typedef struct {
    bool connected;
} WIFI_CONNECT_RESPONSE;

typedef struct {
    WIFI_REQUEST_TYPE type;
    void (*cb)(void* params, void* response);
    void *cb_params;
    union {
        WIFI_GET_ARGS get;
        WIFI_SYNC_TIME_ARGS sync_time;
    };
} WIFI_REQUEST;


void wifi_task_start(void);
MessageBufferHandle_t wifi_message_buffer(void);

#endif //EPAPER_DISPLAY_WIFI_TASK_H
