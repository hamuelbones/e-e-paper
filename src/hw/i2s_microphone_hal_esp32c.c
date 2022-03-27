//
// Created by Samuel Jones on 3/26/22.
//

#include "i2s_microphone_hal.h"
#include "init_hal.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "filesystem_hal.h"
#include "string.h"

#define I2S_LRC (BUTTON_0)
#define I2S_BCLK (SPI_SCK)
#define I2S_DATA (SPI_MISO)
#define I2S_SHUTDOWN (BUTTON_1)

static short data_buf[4096*4];
const size_t WAVE_HEADER_SIZE = 44;

void generate_wav_header(char* wav_header, uint32_t wav_size, uint32_t sample_rate){

    // See this for reference: http://soundfile.sapp.org/doc/WaveFormat/
    uint32_t file_size = wav_size + WAVE_HEADER_SIZE - 8;
    uint32_t byte_rate = 22050*2;

    const char set_wav_header[] = {
            'R','I','F','F', // ChunkID
            file_size, file_size >> 8, file_size >> 16, file_size >> 24, // ChunkSize
            'W','A','V','E', // Format
            'f','m','t',' ', // Subchunk1ID
            0x10, 0x00, 0x00, 0x00, // Subchunk1Size (16 for PCM)
            0x01, 0x00, // AudioFormat (1 for PCM)
            0x01, 0x00, // NumChannels (1 channel)
            sample_rate, sample_rate >> 8, sample_rate >> 16, sample_rate >> 24, // SampleRate
            byte_rate, byte_rate >> 8, byte_rate >> 16, byte_rate >> 24, // ByteRate
            0x02, 0x00, // BlockAlign
            0x10, 0x00, // BitsPerSample (16 bits)
            'd','a','t','a', // Subchunk2ID
            wav_size, wav_size >> 8, wav_size >> 16, wav_size >> 24, // Subchunk2Size
    };

    memcpy(wav_header, set_wav_header, sizeof(set_wav_header));
}


void microphone_record_wav(const char* filename, bool (*until)(void*), void** arg) {

    const int i2s_port = 0;
    app_yield_spi_bus();

    gpio_config_t i2s_data_pin = {
            .intr_type = 0,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = 1<<I2S_DATA,
            .pull_down_en = 0,
            .pull_up_en = 0,
    };
    gpio_config(&i2s_data_pin);

    // Speaker left channel, microphone right channel.
    gpio_set_level(I2S_SHUTDOWN, 1);
    esp_rom_delay_us(100);

    fs_remove(filename);
    file_handle f = fs_open(filename, "wb");
    if (!f) {
        printf("failed to open recording dat file");
        gpio_set_level(I2S_SHUTDOWN, 0);
        app_resume_spi_bus();
        return;
    }

    // Try microphone
    i2s_config_t mic_config = {
            .mode = I2S_MODE_MASTER | I2S_MODE_RX,
            .communication_format = I2S_COMM_FORMAT_STAND_MSB,
            .sample_rate = 22050,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
            .tx_desc_auto_clear = false,
            .dma_desc_num = 16,
            .dma_frame_num = 512,
            .bits_per_chan = I2S_BITS_PER_CHAN_32BIT,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
            .left_align = true,
    };
    i2s_driver_install(i2s_port, &mic_config, 5, NULL);
    i2s_pin_config_t in_pin_config = {
            .mck_io_num = I2S_PIN_NO_CHANGE,
            .bck_io_num = I2S_BCLK,
            .ws_io_num = I2S_LRC,
            .data_in_num = I2S_DATA,
            .data_out_num = I2S_PIN_NO_CHANGE,
    };
    i2s_set_pin(i2s_port, &in_pin_config);
    //i2s_set_clk(i2s_port, 22050, I2S_BITS_PER_SAMPLE_16BIT | (I2S_BITS_PER_CHAN_32BIT<<16), I2S_CHANNEL_MONO);
    i2s_start(i2s_port);

    bool first = true;
    size_t length = 0;
    int lpf;

    char header[WAVE_HEADER_SIZE];
    // Size here is definitely wrong. TODO fix after recording
    generate_wav_header(header, 1, 22050);
    fs_write(f, header, (int)WAVE_HEADER_SIZE);
    do {
        size_t bytes_read;
        i2s_read(i2s_port, data_buf, sizeof(data_buf), &bytes_read, portMAX_DELAY);
        printf("I2s bytes read: %u\n", bytes_read);

        // We need to read both channels... so half the data is nonsense
        for (int i=1; i<sizeof(data_buf)/sizeof(short); i+=2) {
            data_buf[i/2] = data_buf[i];
            data_buf[i] = 0;
        }

        int process_index = 0;
        if (first) {
            // ignore first 30 samples
            process_index = 30;
            lpf = data_buf[30] * 8;
            first = false;
        }

        for(int i=process_index; i < (sizeof(data_buf)/2)/sizeof(short); i++) {
            lpf = lpf * 7 / 8;
            lpf += data_buf[i];
            data_buf[i] = (short) (data_buf[i] * 8 - lpf);
            length += 1;
        }

        fs_write(f, &data_buf[process_index], sizeof(data_buf)/2 - process_index*2);
        printf("samples: %d\n", length);

    } while (!until(*arg) && (length < 22050*10));

    i2s_driver_uninstall(i2s_port);
    gpio_set_level(I2S_SHUTDOWN, 0);
    app_resume_spi_bus();
    fs_close(f);

}
