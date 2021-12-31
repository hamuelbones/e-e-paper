//
// Created by Samuel Jones on 12/30/21.
//

#include <stdint.h>
#include "display_buffer.h"

#ifndef EPAPER_DISPLAY_DISPLAY_DRAW_IMAGE_H
#define EPAPER_DISPLAY_DISPLAY_DRAW_IMAGE_H

void DISPBUF_DrawBitmap(DISPLAY_COORD cursor, uint16_t width, uint16_t height, const uint8_t *bitmap);

#endif //EPAPER_DISPLAY_DISPLAY_DRAW_IMAGE_H
