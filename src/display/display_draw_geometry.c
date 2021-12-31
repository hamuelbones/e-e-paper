//
// Created by Samuel Jones on 12/30/21.
//

#include "display_draw_geometry.h"
#include "display_buffer.h"

void DISPBUF_DrawPoint(uint16_t x, uint16_t y) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return;
    }

    DISPBUF_ActiveBuffer()[100*y + x/8] |= 1<< (7-x%8);
}

void DISPBUF_ClearPoint(uint16_t x, uint16_t y) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return;
    }
    DISPBUF_ActiveBuffer()[100*y + x/8] &= ~(1 << (7-x%8));
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
