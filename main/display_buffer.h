//
// Created by Samuel Jones on 12/9/21.
//

#ifndef EPAPER_DISPLAY_DISPLAY_BUFFER_H
#define EPAPER_DISPLAY_DISPLAY_BUFFER_H

#include <stdint.h>

#define DISPLAY_WIDTH 800
#define DISPLAY_HEIGHT 480

#define BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)

void DISPBUF_Swap(void);
void DISPBUF_ClearActive(void);

uint8_t * DISPBUF_ActiveBuffer(void);
uint8_t * DISPBUF_InactiveBuffer(void);

void DISPBUF_DrawPoint(uint16_t x, uint16_t y);
void DISPBUF_DrawHorizontalLine(uint16_t x, uint16_t y1, uint16_t y2);
void DISPBUF_DrawVerticalLine(uint16_t y, uint16_t x1, uint16_t x2);

#endif //EPAPER_DISPLAY_DISPLAY_BUFFER_H
