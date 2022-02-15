//
// Created by Samuel Jones on 12/30/21.
//

#include <stdint.h>
#include "display_buffer.h"

#ifndef EPAPER_DISPLAY_DISPLAY_DRAW_IMAGE_H
#define EPAPER_DISPLAY_DISPLAY_DRAW_IMAGE_H

void dispbuf_draw_bitmap(DISPLAY_COORD cursor,
                        DISPLAY_COORD bitmap_size,
                        DISPLAY_COORD bitmap_bounds,
                        const uint8_t *bitmap,
                        DRAW_FLAGS flags);

#endif //EPAPER_DISPLAY_DISPLAY_DRAW_IMAGE_H
