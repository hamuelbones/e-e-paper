//
// Created by Samuel Jones on 12/30/21.
//

#include "display_draw_geometry.h"
#include "display_buffer.h"
#include <stdlib.h>

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


void draw_point_with_dilation(DISPLAY_COORD point, int dilation) {

    dispbuf_draw_point(point.x, point.y);

    if (!dilation) {
        return;
    }

    if (dilation >= 2) {
        DISPLAY_COORD next_point = {point.x, point.y+1};
        draw_point_with_dilation(next_point, dilation-2);
        next_point.y = point.y-1;
        draw_point_with_dilation(next_point, dilation-2);
        next_point.y = point.y;
        next_point.x = point.x-1;
        draw_point_with_dilation(next_point, dilation-2);
        next_point.x = point.x+1;
        draw_point_with_dilation(next_point, dilation-2);
    } else {
        DISPLAY_COORD next_point = {point.x, point.y-1};
        draw_point_with_dilation(next_point, 0);
        next_point.x = point.x+1;
        next_point.y = point.y;
        draw_point_with_dilation(next_point, 0);
    }
}

void dispbuf_draw_line(DISPLAY_COORD p0, DISPLAY_COORD p1, uint16_t thickness) {
    if (!thickness) {
        return;
    }
    if ((p0.x == p1.x) && (p0.y == p1.y)) {
        return;
    }

    printf("Draw line from %u, %u to %u, %u, thickness %u\n", p0.x, p0.y, p1.x, p1.y, thickness);

    // This is Bresenham's line drawing algorithm
    int dx = abs(p0.x - p1.x);
    int sx = p0.x < p1.x ? 1 : -1;
    int dy = -abs(p1.y - p0.y);
    int sy = p0.y < p1.y ? 1 : -1;
    int error = dx + dy;

    while (true) {
        draw_point_with_dilation(p0, thickness-1);
        if ((p0.x == p1.x) && (p0.y == p1.y)) {
            break;
        }
        int e2 = 2*error;
        if (e2 >= dy) {
            if (p0.x == p1.x) {
                break;
            }
            error = error + dy;
            p0.x = p0.x + sx;
        }
        if (e2 <= dx) {
            if (p0.y == p1.y) {
                break;
            }
            error = error + dx;
            p0.y = p0.y + sy;
        }
    }

}


void dispbuf_draw_rect_line(DISPLAY_COORD upper_left, DISPLAY_COORD bottom_right, int thickness, bool filled, bool notched) {
    for (int i=0; i<thickness; i++) {

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
        dispbuf_draw_point(upper_left.x + thickness, upper_left.y + thickness);
        dispbuf_draw_point(upper_left.x + thickness, bottom_right.y - thickness);
        dispbuf_draw_point(bottom_right.x - thickness, upper_left.y + thickness);
        dispbuf_draw_point(bottom_right.x - thickness, bottom_right.y - thickness);
    }

    if (filled) {
        for (int i=upper_left.y + thickness; i<=bottom_right.y - thickness; i++) {
            dispbuf_draw_horizontal_line(i, upper_left.x+thickness, bottom_right.y-thickness+1);
        }
    }
}