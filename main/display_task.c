//
// Created by Samuel Jones on 11/26/21.
//

#include "display_task.h"
#include "display_buffer.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define GPIO_DISPLAY_CS (GPIO_NUM_0)
#define GPIO_DISPLAY_SCK (GPIO_NUM_1)
#define GPIO_DISPLAY_MOSI (GPIO_NUM_2)
#define GPIO_DISPLAY_MISO (GPIO_NUM_3)

#define GPIO_DISPLAY_DC (GPIO_NUM_4)
#define GPIO_DISPLAY_RST (GPIO_NUM_5)
#define GPIO_DISPLAY_BUSY (GPIO_NUM_6)

static TaskHandle_t _displayHandle;

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


static void _render(spi_device_handle_t handle, const uint8_t *buffer) {

    printf("Reset the EPD driver IC\n");
    gpio_set_level(GPIO_DISPLAY_RST, 0);
    vTaskDelay(2); // at least 5ms
    gpio_set_level(GPIO_DISPLAY_RST, 1);
    gpio_set_level(GPIO_DISPLAY_DC, 0);
    vTaskDelay(3); // at least 20ms
    while (!gpio_get_level(GPIO_DISPLAY_BUSY))  {vTaskDelay(1);}

    printf("power setting\n");
    const uint8_t power_setting[6] = {0x01, 0x17, 0x17, 0x3f, 0x3f, 0x11};
    _write_display_command(handle, power_setting, 6);

    printf("VCOM DC setting\n");
    const uint8_t vcom_dc_setting[2] = {0x82, 0x24};
    _write_display_command(handle, vcom_dc_setting, 2);

    printf("Booster Setting\n");
    const uint8_t booster_setting[5] = {0x06, 0x27, 0x27, 0x2f, 0x17};
    _write_display_command(handle, booster_setting, 5);

    printf("power on\n");
    const uint8_t power_on[1] = {0x04};
    _write_display_command(handle, power_on, 1);
    vTaskDelay(2); // at least 10ms
    while (!gpio_get_level(GPIO_DISPLAY_BUSY))  {vTaskDelay(1);}

    printf("panel setting\n");
    const uint8_t panel_setting[2] = {0x00, 0x3f};
    _write_display_command(handle, panel_setting, 2);

    printf("tres setting\n");
    const uint8_t tres_setting[5] = {0x61, 0x03, 0x20, 0x01, 0xe0};
    _write_display_command(handle, tres_setting, 5);

    const uint8_t setting[2] = {0x15, 0x00};
    _write_display_command(handle, setting, 2);

    printf("vcom and data interval\n");
    const uint8_t vcom_data_interval_setting[3] = {0x50, 0x10, 0x00};
    _write_display_command(handle, vcom_data_interval_setting, 3);

    printf("tcon setting\n");
    const uint8_t tcon_setting[2] = {0x60, 0x22};
    _write_display_command(handle, tcon_setting, 2);

    printf("resolution setting\n");
    const uint8_t resolution_setting[4] = {0x65, 0x00, 0x00, 0x00};
    _write_display_command(handle, resolution_setting, 4);

    printf("lut?\n");
    const uint8_t lut20[43] = {
            0x20,
            0x0,	0xF,	0xF,	0x0,	0x0,	0x1,
            0x0,	0xF,	0x1,	0xF,	0x1,	0x2,
            0x0,	0xF,	0xF,	0x0,	0x0,	0x1,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
    };
    _write_display_command(handle, lut20, 43);
    const uint8_t  lut21[43] =
            {0x21,
             0x10,	0xF,	0xF,	0x0,	0x0,	0x1,
             0x84,	0xF,	0x1,	0xF,	0x1,	0x2,
             0x20,	0xF,	0xF,	0x0,	0x0,	0x1,
             0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
             0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
             0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
             0x0,	0x0,	0x0,	0x0,	0x0,	0x0};

    _write_display_command(handle, lut21, 43);
    const uint8_t lut22[43] ={
            0x22,
            0x10,	0xF,	0xF,	0x0,	0x0,	0x1,
            0x84,	0xF,	0x1,	0xF,	0x1,	0x2,
            0x20,	0xF,	0xF,	0x0,	0x0,	0x1,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0};

    _write_display_command(handle, lut22, 43);

    const uint8_t lut23[43] ={
            0x23,
            0x80,	0xF,	0xF,	0x0,	0x0,	0x1,
            0x84,	0xF,	0x1,	0xF,	0x1,	0x2,
            0x40,	0xF,	0xF,	0x0,	0x0,	0x1,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0
    };

    _write_display_command(handle, lut23, 43);
    const uint8_t lut24[43] = {
            0x24,
            0x80,	0xF,	0xF,	0x0,	0x0,	0x1,
            0x84,	0xF,	0x1,	0xF,	0x1,	0x2,
            0x40,	0xF,	0xF,	0x0,	0x0,	0x1,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
            0x0,	0x0,	0x0,	0x0,	0x0,	0x0
    };
    _write_display_command(handle, lut24, 43);


    printf("image data\n");

    const uint8_t new_data[1] = {0x13};
    _write_display_command(handle, new_data, 1);

    gpio_set_level(GPIO_DISPLAY_DC, 1);

    for (int i=0; i<BUFFER_SIZE; i+=256) {
        spi_transaction_t transaction = {
                .length=256*8, .tx_buffer=&buffer[i],
        };
        spi_device_polling_transmit(handle, &transaction);
    }

    gpio_set_level(GPIO_DISPLAY_DC, 0);

    vTaskDelay(2); // at least 10ms
    while (!gpio_get_level(GPIO_DISPLAY_BUSY))  {vTaskDelay(1);}

    printf("display refresh\n");

    const uint8_t refresh[1] = {0x12};
    _write_display_command(handle, refresh, 1);
    vTaskDelay(2); // at least 10ms
    while (!gpio_get_level(GPIO_DISPLAY_BUSY))  {vTaskDelay(1);}

    while (!gpio_get_level(GPIO_DISPLAY_BUSY))  {vTaskDelay(1);}

    printf("power_off\n");

    const uint8_t power_off[1] = {0x02};
    _write_display_command(handle, power_off, 1);

    const uint8_t deep_sleep[2] = {0x07, 0xa5};
    _write_display_command(handle, deep_sleep, 2);

}


void _Noreturn display_task(void* params) {

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

    spi_bus_config_t busConfig = {
            .miso_io_num = GPIO_DISPLAY_MISO,
            .mosi_io_num = GPIO_DISPLAY_MOSI,
            .sclk_io_num = GPIO_DISPLAY_SCK,
            .quadhd_io_num = -1,
            .quadwp_io_num = -1,
            .max_transfer_sz = 5000,
    };

    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &busConfig, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t deviceConfig = {
            .mode = 0,
            .clock_speed_hz = 1*1000*1000,
            .spics_io_num = GPIO_DISPLAY_CS,
            .queue_size = 4,
    };

    spi_device_handle_t deviceHandle;

    ret = spi_bus_add_device(SPI2_HOST, &deviceConfig, &deviceHandle);
    ESP_ERROR_CHECK(ret);

    static int i = 0;

    while (1) {
        vTaskDelay(10);
        printf("next frame %i\n", i);

        char test_string[100];
        snprintf(test_string, 100, "Hello font-rendering world! %u %u Happy wedding Brandon and Kenzie!", i, i);

        DISPBUF_ClearActive();
        DISPBUF_DrawString(5, 100, test_string, HELVETICA_14);

        DISPBUF_DrawString(5, 5, "TEST", HELVETICA_14);
        DISPBUF_DrawString(400, 5, "TESTX", HELVETICA_14);
        DISPBUF_DrawString(400, 400, "TESTXY", HELVETICA_14);
        DISPBUF_DrawString(5, 400, "TESTY", HELVETICA_14);

        DISPBUF_DrawHorizontalLine(200, 200, 400);

        _render(deviceHandle, DISPBUF_ActiveBuffer());
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