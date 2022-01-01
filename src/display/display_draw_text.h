//
// Created by Samuel Jones on 12/30/21.
//

#include <stdint.h>
#include "display_buffer.h"

#ifndef EPAPER_DISPLAY_DISPLAY_DRAW_TEXT_H
#define EPAPER_DISPLAY_DISPLAY_DRAW_TEXT_H

typedef enum {
    JUSTIFICATION_LEFT,
    JUSTIFICATION_CENTER,
    JUSTIFICATION_RIGHT,
} STRING_JUSTIFICATION;

typedef enum {
    DRAW_TEXT_MEASURE = (1<<0),
    DRAW_TEXT_JUSTIFY_CENTER = (1<<1),
    DRAW_TEXT_JUSTIFY_RIGHT = (1<<2),
} DRAW_TEXT_FLAGS;

int DISPBUF_DrawCharacter(DISPLAY_COORD cursor, char c, EPAPER_DISPLAY_FONT_ID fontId, DRAW_TEXT_FLAGS flags);
int DISPBUF_DrawLabel(DISPLAY_COORD cursor, const char*s, EPAPER_DISPLAY_FONT_ID font_id, DRAW_TEXT_FLAGS flags);

int DISPBUF_DrawMultiline(DISPLAY_COORD cursor, const char *s, EPAPER_DISPLAY_FONT_ID font_id,
                          uint16_t max_x, uint16_t max_y, DRAW_TEXT_FLAGS flags);

#endif //EPAPER_DISPLAY_DISPLAY_DRAW_TEXT_H
