//
// Created by Samuel Jones on 12/9/21.
//

#include "display_buffer.h"
#include <string.h>

static uint8_t _buffer1[BUFFER_SIZE];

static uint8_t* _activeBuf = _buffer1;

void DISPBUF_ClearActive(void) {
    memset(_activeBuf, 0x00, BUFFER_SIZE);
}

uint8_t * DISPBUF_ActiveBuffer(void) {
    return _activeBuf;
}

void DISPBUF_DrawPoint(uint16_t x, uint16_t y) {
    _activeBuf[100*x + y/8] |= 1<<(7-y%8);
}

void DISPBUF_DrawHorizontalLine(uint16_t x, uint16_t y1, uint16_t y2) {
    for (int y=y1; y<y2; y++) {
        DISPBUF_DrawPoint(x, y);
    }
}

void DISPBUF_DrawVerticalLine(uint16_t y, uint16_t x1, uint16_t x2) {
    for (int x=x1; x<x2; x++) {
        DISPBUF_DrawPoint(x, y);
    }
}