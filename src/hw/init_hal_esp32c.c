
#include <stdarg.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_pm.h"

#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"


#define GPIO_CHG_EN (GPIO_NUM_4)
#define GPIO_CHG_PGOOD (GPIO_NUM_7)
#define GPIO_CHG_STAT1 (GPIO_NUM_5)
#define GPIO_CHG_STAT2 (GPIO_NUM_6)

#define SPI_SCK (GPIO_NUM_0)
#define SPI_MOSI (GPIO_NUM_1)
#define SPI_MISO (GPIO_NUM_2)

#define BUTTON (GPIO_NUM_10)

void app_hal_init(void) {

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

    gpio_config_t button = {
        .intr_type = 0,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1<<BUTTON,
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&button);
    if (gpio_get_level(BUTTON) == 0) {
        for (int i=0; i<120; i++) {
            printf("Waiting for flash... %i\n", i);
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
    }

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

    // Init SPI bus
    spi_bus_config_t busConfig = {
            .miso_io_num = SPI_MISO,
            .mosi_io_num = SPI_MOSI,
            .sclk_io_num = SPI_SCK,
            .quadhd_io_num = -1,
            .quadwp_io_num = -1,
            .max_transfer_sz = 5000,
    };

    ret = spi_bus_initialize(SPI2_HOST, &busConfig, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    esp_pm_config_esp32c3_t power_config;
    power_config.light_sleep_enable = true;
    power_config.min_freq_mhz = 40;
    power_config.max_freq_mhz = CONFIG_ESP32C3_DEFAULT_CPU_FREQ_MHZ;
    esp_pm_configure(&power_config);

}

void app_hal_reboot(void) {
    esp_restart();
}