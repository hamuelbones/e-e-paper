//
// Created by Samuel Jones on 12/9/21.
//

#include "display_buffer.h"
#include <string.h>
#include "fonts.h"
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include "freertos/FreeRTOS.h"


static uint8_t *_buffer1;
static uint8_t *_buffer2;
static uint16_t _width;
static uint16_t _height;

static uint8_t* _activeBuf = NULL;

void dispbuf_init(uint16_t width, uint16_t height) {
    _width = width;
    _height = height;
    _buffer1 = pvPortMalloc(_width * _height / 8);
    _buffer2 = pvPortMalloc(_width * _height / 8);
    _activeBuf = _buffer1;
}

DISPLAY_COORD dispbuf_dims(void) {
    DISPLAY_COORD coord = {
        .x = _width,
        .y = _height,
    };
    return coord;
}

void dispbuf_clear_active(void) {
    memset(_activeBuf, 0x00, _width * _height / 8);
}

uint8_t * dispbuf_active_buffer(void) {
    return _activeBuf;
}

uint8_t * dispbuf_inactive_buffer(void) {
    return _activeBuf == _buffer1 ? _buffer2 : _buffer1;
}

void dispbuf_swap(void) {
    if (_activeBuf == _buffer1) {
        _activeBuf = _buffer2;
    } else {
        _activeBuf = _buffer1;
    }
}
