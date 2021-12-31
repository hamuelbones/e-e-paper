//
// Created by Samuel Jones on 11/26/21.
//

#include "wifi_task.h"
#include "wifi_hal.h"

static TaskHandle_t _wifiHandle;


void _Noreturn wifi_task(void* params) {

    vTaskDelay(1000);

    WIFI_Init();
    WIFI_Connect("ham", "smokyradio");

    while (1) {
        vTaskDelay(1000 * 1000 / configTICK_RATE_HZ);
    }
}

TaskHandle_t wifi_task_handle(void) {
    return _wifiHandle;
}


void wifi_task_start(void) {
    xTaskCreate(wifi_task,
                "WIFI",
                4096,
                NULL,
                5,
                &_wifiHandle);
}