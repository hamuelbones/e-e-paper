//
// Created by Samuel Jones on 12/9/21.
//

#include "display_buffer.h"
#include <string.h>
#include "font/fonts.h"
#include <stdio.h>

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

void DISPBUF_DrawPoint(uint16_t x, uint16_t y) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return;
    }
    _activeBuf[100*y + x/8] |= 1<< (7-x%8);
}

void DISPBUF_ClearPoint(uint16_t x, uint16_t y) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return;
    }
    _activeBuf[100*y + x/8] &= ~(1 << (7-x%8));
}

void DISPBUF_DrawVerticalLine(uint16_t x, uint16_t y1, uint16_t y2) {
    for (int y=y1; y<y2; y++) {
        DISPBUF_DrawPoint(x, y);
    }
}

void DISPBUF_DrawHorizontalLine(uint16_t y, uint16_t x1, uint16_t x2) {
    for (int x=x1; x<x2; x++) {
        DISPBUF_DrawPoint(x, y);
    }
}

void DISPBUF_DrawBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *bitmap) {
    for(int j=y; j<y+height; j++) {
        int offset = ((width + 7) / 8)*(j-y);
        for (int i=x; i<x+width; i++) {
            if (bitmap[offset+(i-x)/8] & (1<<(7-(i-x)%8))) {
                DISPBUF_DrawPoint(i, j);
            } else {
                DISPBUF_ClearPoint(i, j);
            }
        }
    }
}

int DISPBUF_DrawCharacter(uint16_t x, uint16_t y, char c, EPAPER_DISPLAY_FONT_ID fontId) {
    const FONT_CHARACTER *bitmap = FONT_GetBitmap(fontId, c);
    if (!bitmap) {
        return 0;
    }
    DISPBUF_DrawBitmap(x, y, bitmap->width, bitmap->height, bitmap->data);
    return bitmap->width;
}

int DISPBUF_DrawString(uint16_t x, uint16_t y, const char* s, EPAPER_DISPLAY_FONT_ID fontId) {
    int offset = 0;
    printf("%s\n", s);
    while(*s != '\0') {
        offset += DISPBUF_DrawCharacter(x+offset, y, *s, fontId);
        s++;
    }
    return offset;
}