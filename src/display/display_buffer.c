//
// Created by Samuel Jones on 12/9/21.
//

#include "display_buffer.h"
#include <string.h>
#include "fonts.h"
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>


static uint8_t _buffer1[BUFFER_SIZE];
static uint8_t _buffer2[BUFFER_SIZE];

static uint8_t* _activeBuf = _buffer1;

void DISPBUF_ClearActive(void) {
    memset(_activeBuf, 0x00, BUFFER_SIZE);
}

uint8_t * DISPBUF_ActiveBuffer(void) {
    return _activeBuf;
}

uint8_t * DISPBUF_InactiveBuffer(void) {
    return _activeBuf == _buffer1 ? _buffer2 : _buffer1;
}

void DISPBUF_Swap(void) {
    if (_activeBuf == _buffer1) {
        _activeBuf = _buffer2;
    } else {
        _activeBuf = _buffer1;
    }
}
