//
// Created by Samuel Jones on 3/26/22.
//

#include "i2s_speaker_hal.h"
#include "init_hal.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "filesystem_hal.h"

#define I2S_LRC (BUTTON_0)
#define I2S_BCLK (SPI_SCK)
#define I2S_DATA (SPI_MISO)
#define I2S_SHUTDOWN (BUTTON_1)

static short data_buf[2048*4];

void speaker_play_wav(const char* filename, bool (*until)(void*), void** arg) {



    const int i2s_port = 0;
    app_yield_spi_bus();

    gpio_config_t i2s_data_pin = {
            .intr_type = 0,
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1<<I2S_DATA,
            .pull_down_en = 0,
            .pull_up_en = 0,
    };
    gpio_config(&i2s_data_pin);

    gpio_set_level(I2S_SHUTDOWN, 1);
    esp_rom_delay_us(10);

    i2s_config_t i2s_config = {
            .mode = I2S_MODE_MASTER | I2S_MODE_TX,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .sample_rate = 22050,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .tx_desc_auto_clear = false,
            .dma_desc_num = 8,
            .dma_frame_num = 256,
            .bits_per_chan = 0,
    };
    i2s_driver_install(i2s_port, &i2s_config, 5, NULL);

    i2s_pin_config_t pin_config = {
            .mck_io_num = I2S_PIN_NO_CHANGE,
            .bck_io_num = I2S_BCLK,
            .ws_io_num = I2S_LRC,
            .data_out_num = I2S_DATA,
            .data_in_num = I2S_PIN_NO_CHANGE,
    };
    i2s_set_pin(i2s_port, &pin_config);
    i2s_start(i2s_port);

    file_handle f = fs_open(filename, "rb");
    if (!f) {
        printf("No saved file :(\n");
        i2s_driver_uninstall(i2s_port);
        gpio_set_level(I2S_SHUTDOWN, 0);
        app_resume_spi_bus();
        return;
    }
    fs_fseek(f, 44, SEEK_SET); // skip wav header

    do {
        size_t len = fs_read(f, data_buf, sizeof(data_buf));

        size_t bytes_written = 0;
        i2s_write(i2s_port, data_buf, len, &bytes_written, portMAX_DELAY);
        printf("I2s bytes written: %u\n", bytes_written);

    } while (!until(*arg) && !fs_feof(f));
    fs_close(f);

    i2s_driver_uninstall(i2s_port);

    gpio_set_level(I2S_SHUTDOWN, 0);
    app_resume_spi_bus();
}
