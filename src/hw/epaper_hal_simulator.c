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

#define CANVAS_HEIGHT 480
#define CANVAS_WIDTH 800
uint8_t gtk_buffer[CANVAS_HEIGHT * CANVAS_WIDTH * 3];

void epaper_init(void) {
}

void epaper_render_buffer(const uint8_t *buffer, const uint8_t *old_buffer, size_t buffer_size) {
    for (int i=0; i<buffer_size*8; i++) {
        uint8_t val;
        if (buffer[i/8] & (1<<(7-i%8))) {
            val = 0;
        } else {
            val = 0xFF;
        }
        gtk_buffer[i*3] = val;
        gtk_buffer[i*3+1] = val;
        gtk_buffer[i*3+2] = val;
    }
}


void app_hal_reboot(void) {
    exit(0);
}