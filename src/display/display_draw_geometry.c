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


void DISPBUF_DrawRect(DISPLAY_COORD upper_left, DISPLAY_COORD bottom_right, int width, bool filled, bool notched) {
    for (int i=0; i<width; i++) {

        DISPBUF_DrawHorizontalLine(upper_left.y+i, upper_left.x, bottom_right.x);
        DISPBUF_DrawHorizontalLine(bottom_right.y-i, upper_left.x, bottom_right.x);
        DISPBUF_DrawVerticalLine(upper_left.x+i, upper_left.y, bottom_right.y);
        DISPBUF_DrawVerticalLine(bottom_right.x-i, upper_left.y, bottom_right.y);

        if (i==0 && notched) {
            DISPBUF_ClearPoint(upper_left.x, upper_left.y);
            DISPBUF_ClearPoint(upper_left.x, bottom_right.y);
            DISPBUF_ClearPoint(bottom_right.x, upper_left.y);
            DISPBUF_ClearPoint(bottom_right.x, bottom_right.y);
        }
    }

    if (notched) {
        DISPBUF_DrawPoint(upper_left.x + width, upper_left.y + width);
        DISPBUF_DrawPoint(upper_left.x + width, bottom_right.y - width);
        DISPBUF_DrawPoint(bottom_right.x - width, upper_left.y + width);
        DISPBUF_DrawPoint(bottom_right.x - width, bottom_right.y - width);
    }

    if (filled) {
        for (int i=upper_left.y + width; i<=bottom_right.y - width; i++) {
            DISPBUF_DrawHorizontalLine(i, upper_left.x+width, bottom_right.y-width+1);
        }
    }
}