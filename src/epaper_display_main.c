/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wifi_task.h"
#include "display_task.h"
#include "bluetooth_task.h"

#include "init_hal.h"


void _Noreturn app_main(void)
{
    HAL_Init();

    wifi_task_start();
    vTaskDelay(2);
    display_task_start();
    vTaskDelay(2);

    while (1) {
        HAL_Print("Hi from main\n");
        vTaskDelay(1000 * 1000 / configTICK_RATE_HZ);
    }
}
