//
// Created by Samuel Jones on 12/9/21.
//

#ifndef EPAPER_DISPLAY_DISPLAY_BUFFER_H
#define EPAPER_DISPLAY_DISPLAY_BUFFER_H

#include <stdint.h>
#include "fonts.h"
#include <stdbool.h>

typedef struct {
    uint16_t x;
    uint16_t y;
} DISPLAY_COORD;

#define DISPLAY_WIDTH 800
#define DISPLAY_HEIGHT 480

#define BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)

void DISPBUF_Swap(void);
void DISPBUF_ClearActive(void);

uint8_t * DISPBUF_ActiveBuffer(void);
uint8_t * DISPBUF_InactiveBuffer(void);


#endif //EPAPER_DISPLAY_DISPLAY_BUFFER_H
