//
// Created by Samuel Jones on 12/30/21.
//


#include <stdint.h>
#include "display_buffer.h"

#ifndef EPAPER_DISPLAY_DISPLAY_DRAW_GEOMETRY_H
#define EPAPER_DISPLAY_DISPLAY_DRAW_GEOMETRY_H

void DISPBUF_DrawPoint(uint16_t x, uint16_t y);
void DISPBUF_ClearPoint(uint16_t x, uint16_t y);
void DISPBUF_DrawHorizontalLine(uint16_t y, uint16_t x1, uint16_t x2);
void DISPBUF_DrawVerticalLine(uint16_t x, uint16_t y1, uint16_t y2);


#endif //EPAPER_DISPLAY_DISPLAY_DRAW_GEOMETRY_H
