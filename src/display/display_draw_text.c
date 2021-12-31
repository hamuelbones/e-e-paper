//
// Created by Samuel Jones on 12/30/21.
//

#include "display_draw_text.h"
#include "display_draw_image.h"

#include <ctype.h>

static bool DISPBUF_IsWhitespace(char c) {
    return isspace(c);
}

static bool DISPBUF_IsNewline(char c) {
    return (c == '\n');
}

int DISPBUF_CharacterWidth(EPAPER_DISPLAY_FONT_ID fontId, char c) {
    const FONT_CHARACTER *bitmap = FONT_GetBitmap(fontId, c);
    if (!bitmap) {
        return 0;
    }
    return bitmap->width;
}

int DISPBUF_DrawCharacter(DISPLAY_COORD cursor, char c, EPAPER_DISPLAY_FONT_ID fontId, DRAW_TEXT_FLAGS flags) {
    const FONT_CHARACTER *bitmap = FONT_GetBitmap(fontId, c);
    if (!bitmap) {
        return 0;
    }
    if (!(flags & DRAW_TEXT_MEASURE)) {
        DISPBUF_DrawBitmap(cursor, bitmap->width, bitmap->height, bitmap->data);
    }
    return bitmap->width;
}

int DISPBUF_DrawLabel(DISPLAY_COORD cursor, const char*s, EPAPER_DISPLAY_FONT_ID font_id) {
    int cur_offset = 0;

    while (*s != 0) {
        int char_width = DISPBUF_DrawCharacter(cursor, *s, font_id, false);
        cur_offset += char_width;
        cursor.x += char_width;
        s++;
    }

    return cur_offset;
}
