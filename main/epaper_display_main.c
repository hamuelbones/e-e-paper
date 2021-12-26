/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "nvs_flash.h"
#include "driver/gpio.h"

#include "wifi_task.h"
#include "display_task.h"
#include "bluetooth_task.h"

#define GPIO_CHG_EN (GPIO_NUM_4)
#define GPIO_CHG_PGOOD (GPIO_NUM_7)
#define GPIO_CHG_STAT1 (GPIO_NUM_5)
#define GPIO_CHG_STAT2 (GPIO_NUM_6)

void _Noreturn app_main(void)
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    gpio_config_t chg_en = {
            .intr_type = 0,
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1<<GPIO_CHG_EN,
            .pull_down_en = 0,
            .pull_up_en = 0,
    };
    gpio_config(&chg_en);
    gpio_set_level(GPIO_CHG_EN, 0);

    gpio_config_t chg_input = {
            .intr_type = 0,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = (1<<GPIO_CHG_PGOOD) | (1<<GPIO_CHG_STAT1) | (1<<GPIO_CHG_STAT2),
            .pull_down_en = 0,
            .pull_up_en = 1,
    };
    gpio_config(&chg_input);


    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_task_start();
    display_task_start();

    while (1) {
        vTaskDelay(1000);
        printf("Hi from main\n");
    }
}
