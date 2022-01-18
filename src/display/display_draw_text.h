//
// Created by Samuel Jones on 12/30/21.
//

#include <stdint.h>
#include "display_buffer.h"

#ifndef EPAPER_DISPLAY_DISPLAY_DRAW_TEXT_H
#define EPAPER_DISPLAY_DISPLAY_DRAW_TEXT_H


int DISPBUF_DrawCharacter(DISPLAY_COORD cursor, char c, EPAPER_DISPLAY_FONT_ID fontId, DRAW_FLAGS flags);
int DISPBUF_DrawLabel(DISPLAY_COORD cursor, const char*s, EPAPER_DISPLAY_FONT_ID font_id, DRAW_FLAGS flags);

// Returns number of lines drawn
int DISPBUF_DrawMultiline(DISPLAY_COORD cursor, const char *s, EPAPER_DISPLAY_FONT_ID font_id,
                          uint16_t max_x, uint16_t max_y, DRAW_FLAGS flags);

#endif //EPAPER_DISPLAY_DISPLAY_DRAW_TEXT_H
