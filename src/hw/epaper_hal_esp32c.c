//
// Created by Samuel Jones on 12/29/21.
//

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "epaper_hal.h"
#include "epapers/epapers.h"

#define GPIO_DISPLAY_CS (GPIO_NUM_9)

#define GPIO_DISPLAY_DC (GPIO_NUM_3)
#define GPIO_DISPLAY_RST (GPIO_NUM_21)
#define GPIO_DISPLAY_BUSY (GPIO_NUM_20)

static spi_device_handle_t _handle;
static const EPAPER_SPI_HAL_CONFIG* _config;


static void _write_display_command(spi_device_handle_t handle, const uint8_t *cmd, uint8_t len) {
    gpio_set_level(GPIO_DISPLAY_DC, 0);

    int num_transactions = 1;
    spi_transaction_t transactions[2] = {0};

    transactions[0].length = 8;
    transactions[0].tx_buffer = cmd;

    if (len > 1) {
        num_transactions = 2;
        transactions[1].length = (len-1)*8;
        transactions[1].tx_buffer = cmd+1;
    }

    esp_err_t ret = spi_device_polling_transmit(handle, &transactions[0]);
    ESP_ERROR_CHECK(ret);
    if (num_transactions == 2) {
        gpio_set_level(GPIO_DISPLAY_DC, 1);
        ret = spi_device_polling_transmit(handle, &transactions[1]);
        ESP_ERROR_CHECK(ret);
    }
    gpio_set_level(GPIO_DISPLAY_DC, 0);
}

void epaper_init(const EPAPER_SPI_HAL_CONFIG* config) {

    _config = config;
    gpio_deep_sleep_hold_en();

    gpio_config_t dc_rst = {
            .intr_type = 0,
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1<<GPIO_DISPLAY_DC | 1<<GPIO_DISPLAY_RST,
            .pull_down_en = 0,
            .pull_up_en = 0,
    };
    gpio_config(&dc_rst);
    gpio_set_level(GPIO_DISPLAY_DC, 0);

    gpio_set_level(GPIO_DISPLAY_RST, 0);

    gpio_config_t busy = {
            .intr_type = 0,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = 1<<GPIO_DISPLAY_BUSY,
            .pull_down_en = 0,
            .pull_up_en = 1,
    };
    gpio_config(&busy);

    spi_device_interface_config_t deviceConfig = {
            .mode = 0,
            .clock_speed_hz = _config->spi_speed_hz,
            .spics_io_num = GPIO_DISPLAY_CS,
            .queue_size = 4,
    };

    esp_err_t ret = spi_bus_add_device(SPI2_HOST, &deviceConfig, &_handle);
    ESP_ERROR_CHECK(ret);

}

const EPAPER_SPI_HAL_CONFIG* epaper_config(void) {
    return _config;
}


void epaper_run_ops(const EPAPER_HAL_OP *op, const uint8_t *buffer, const uint8_t *old_buffer, size_t buffer_size) {

    bool finished = false;
    while (!finished) {
        switch(op->id) {
            case EPAPER_WAIT_UNTIL_READY:
                while (!gpio_get_level(GPIO_DISPLAY_BUSY)) {
                    vTaskDelay(1);
                }
                break;
            case EPAPER_RESET:
                gpio_set_level(GPIO_DISPLAY_RST, 0);
                vTaskDelay(op->length / portTICK_PERIOD_MS + 1); // at least 5ms
                gpio_set_level(GPIO_DISPLAY_RST, 1);
                gpio_set_level(GPIO_DISPLAY_DC, 0);
                break;
            case EPAPER_SEND_COMMAND:
                _write_display_command(_handle, op->data, op->length);
                break;
            case EPAPER_WRITE_OLD_DATA:
                gpio_set_level(GPIO_DISPLAY_DC, 1);
                for (int i=0; i<buffer_size; i+=256) {
                    spi_transaction_t transaction = {
                            .length=256*8, .tx_buffer=&old_buffer[i],
                    };
                    spi_device_polling_transmit(_handle, &transaction);
                }
                gpio_set_level(GPIO_DISPLAY_DC, 0);
                break;
            case EPAPER_WRITE_NEW_DATA:
                gpio_set_level(GPIO_DISPLAY_DC, 1);
                for (int i=0; i<buffer_size; i+=256) {
                    spi_transaction_t transaction = {
                            .length=256*8, .tx_buffer=&buffer[i],
                    };
                    spi_device_polling_transmit(_handle, &transaction);
                }

                gpio_set_level(GPIO_DISPLAY_DC, 0);
                break;
            case EPAPER_WAIT_TIME:
                vTaskDelay(op->length / portTICK_PERIOD_MS + 1);
                break;
            case EPAPER_COMMAND_MAX:
            default:
                finished = true;
                break;
        }
        op = op + 1;
    }
}


void epaper_render_buffer(const EPAPER_SPI_HAL_CONFIG* config, const uint8_t *buffer, const uint8_t *old_buffer, size_t buffer_size) {

    const EPAPER_HAL_OP *op = config->full_refresh_operation;
    epaper_run_ops(op, buffer, old_buffer, buffer_size);

}