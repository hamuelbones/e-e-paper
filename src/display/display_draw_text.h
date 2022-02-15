//
// Created by Samuel Jones on 12/30/21.
//

#include <stdint.h>
#include "display_buffer.h"

#ifndef EPAPER_DISPLAY_DISPLAY_DRAW_TEXT_H
#define EPAPER_DISPLAY_DISPLAY_DRAW_TEXT_H


int dispbuf_draw_character(DISPLAY_COORD cursor, char c, const FONT_TABLE* font, DRAW_FLAGS flags);
int dispbuf_draw_label(DISPLAY_COORD cursor, const char*s, const FONT_TABLE* font, DRAW_FLAGS flags);

// Returns number of lines drawn
int dispbuf_draw_text(DISPLAY_COORD cursor, const char *s, const FONT_TABLE* font,
                          uint16_t max_x, uint16_t max_y, DRAW_FLAGS flags);

#endif //EPAPER_DISPLAY_DISPLAY_DRAW_TEXT_H
