//
// Created by Samuel Jones on 12/30/21.
//


#include <stdint.h>
#include "display_buffer.h"

#ifndef EPAPER_DISPLAY_DISPLAY_DRAW_GEOMETRY_H
#define EPAPER_DISPLAY_DISPLAY_DRAW_GEOMETRY_H

void dispbuf_draw_point(uint16_t x, uint16_t y);
void dispbuf_clear_point(uint16_t x, uint16_t y);
void dispbuf_draw_horizontal_line(uint16_t y, uint16_t x1, uint16_t x2);
void dispbuf_draw_vertical_line(uint16_t x, uint16_t y1, uint16_t y2);
void dispbuf_draw_line(DISPLAY_COORD p1, DISPLAY_COORD p2, uint16_t thickness);

void dispbuf_draw_rect_line(DISPLAY_COORD upper_left, DISPLAY_COORD bottom_right, int thickness, bool filled, bool notched);


#endif //EPAPER_DISPLAY_DISPLAY_DRAW_GEOMETRY_H
