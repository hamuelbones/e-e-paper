//
// Created by Samuel Jones on 12/9/21.
//

#ifndef EPAPER_DISPLAY_DISPLAY_BUFFER_H
#define EPAPER_DISPLAY_DISPLAY_BUFFER_H

#include <stdint.h>
#include "fonts.h"
#include <stdbool.h>

typedef enum {
    DRAW_MEASURE = (1<<0),
    DRAW_JUSTIFY_HORIZ_CENTER = (1<<1),
    DRAW_JUSTIFY_HORIZ_RIGHT = (1<<2),
    DRAW_JUSTIFY_VERT_CENTER = (1<<3),
    DRAW_JUSTIFY_VERT_BOTTOM = (1<<4),
} DRAW_FLAGS;

typedef struct {
    int16_t x;
    int16_t y;
} DISPLAY_COORD;

void dispbuf_init(uint16_t width, uint16_t height, bool pixels_inverted, bool xy_flipped);
void dispbuf_swap(void);
void dispbuf_clear_active(void);
DISPLAY_COORD dispbuf_dims(void);

bool dispbuf_pixels_inverted(void);
bool dispbuf_xy_flipped(void);

uint8_t * dispbuf_active_buffer(void);
uint8_t * dispbuf_inactive_buffer(void);


#endif //EPAPER_DISPLAY_DISPLAY_BUFFER_H
