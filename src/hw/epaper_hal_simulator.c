//
// Created by Samuel Jones on 12/29/21.
//

#include "init_hal.h"
#include "wifi_hal.h"
#include "epaper_hal.h"
#include "cryptography_hal.h"

#include <stdarg.h>
#include <printf.h>
#include <signal.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <stdbool.h>

void app_hal_init(void) {

}

uint8_t *gtk_buffer;

static const EPAPER_SPI_HAL_CONFIG* _config;

void epaper_init(const EPAPER_SPI_HAL_CONFIG* config) {
    _config = config;
    if (!gtk_buffer) {
        free(gtk_buffer);
    }
    gtk_buffer = calloc(_config->width * _config->height * 3, 1);
}

const EPAPER_SPI_HAL_CONFIG* epaper_config(void) {
    return _config;
}


void epaper_render_buffer(const EPAPER_SPI_HAL_CONFIG* config, const uint8_t *buffer, const uint8_t *old_buffer, size_t buffer_size) {
    for (int i=0; i<buffer_size*8; i++) {

        int bit = i;
        if (_config->xy_flipped) {
            int x = _config->width - i/_config->height;
            int y = i%_config->height;
            bit = y * _config->width + x;
        }

        uint8_t val;
        if (buffer[i/8] & (1<<(7-i%8))) {
            val =  _config->bw_inverted ? 0xFF : 0;
        } else {
            val = _config->bw_inverted ? 0 : 0xFF;
        }
        gtk_buffer[bit*3] = val;
        gtk_buffer[bit*3+1] = val;
        gtk_buffer[bit*3+2] = val;
    }
}

