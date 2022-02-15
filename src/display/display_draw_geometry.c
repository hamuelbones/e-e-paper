//
// Created by Samuel Jones on 12/30/21.
//

#include "display_draw_geometry.h"
#include "display_buffer.h"

void dispbuf_draw_point(uint16_t x, uint16_t y) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return;
    }

    dispbuf_active_buffer()[100*y + x/8] |= 1<< (7-x%8);
}

void dispbuf_clear_point(uint16_t x, uint16_t y) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return;
    }
    dispbuf_active_buffer()[100*y + x/8] &= ~(1 << (7-x%8));
}

void dispbuf_draw_vertical_line(uint16_t x, uint16_t y1, uint16_t y2) {
    for (int y=y1; y<y2; y++) {
        dispbuf_draw_point(x, y);
    }
}

void dispbuf_draw_horizontal_line(uint16_t y, uint16_t x1, uint16_t x2) {
    for (int x=x1; x<x2; x++) {
        dispbuf_draw_point(x, y);
    }
}


void dispbuf_draw_rect(DISPLAY_COORD upper_left, DISPLAY_COORD bottom_right, int width, bool filled, bool notched) {
    for (int i=0; i<width; i++) {

        dispbuf_draw_horizontal_line(upper_left.y+i, upper_left.x, bottom_right.x);
        dispbuf_draw_horizontal_line(bottom_right.y-i, upper_left.x, bottom_right.x);
        dispbuf_draw_vertical_line(upper_left.x+i, upper_left.y, bottom_right.y);
        dispbuf_draw_vertical_line(bottom_right.x-i, upper_left.y, bottom_right.y);

        if (i==0 && notched) {
            dispbuf_clear_point(upper_left.x, upper_left.y);
            dispbuf_clear_point(upper_left.x, bottom_right.y);
            dispbuf_clear_point(bottom_right.x, upper_left.y);
            dispbuf_clear_point(bottom_right.x, bottom_right.y);
        }
    }

    if (notched) {
        dispbuf_draw_point(upper_left.x + width, upper_left.y + width);
        dispbuf_draw_point(upper_left.x + width, bottom_right.y - width);
        dispbuf_draw_point(bottom_right.x - width, upper_left.y + width);
        dispbuf_draw_point(bottom_right.x - width, bottom_right.y - width);
    }

    if (filled) {
        for (int i=upper_left.y + width; i<=bottom_right.y - width; i++) {
            dispbuf_draw_horizontal_line(i, upper_left.x+width, bottom_right.y-width+1);
        }
    }
}