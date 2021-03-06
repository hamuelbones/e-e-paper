//
// Created by Samuel Jones on 12/30/21.
//

#include "display_draw_text.h"
#include "display_draw_image.h"

#include <ctype.h>

static bool DISPBUF_IsWhitespace(char c) {
    return isspace((int)c);
}

static bool DISPBUF_IsNewline(char c) {
    return (c == '\n');
}

static int DISPBUF_FontHeight(const FONT_TABLE* font) {
    const FONT_CHARACTER *bitmap = font_get_bitmap(font, ' ');
    if (!bitmap) {
        return 0;
    }
    return bitmap->height;
}

int dispbuf_draw_character(DISPLAY_COORD cursor, char c, const FONT_TABLE* font, DRAW_FLAGS flags) {
    const FONT_CHARACTER *bitmap = font_get_bitmap(font, c);
    if (!bitmap) {
        return 0;
    }
    if (!(flags & DRAW_MEASURE)) {
        DISPLAY_COORD bitmap_size = {.x=bitmap->width, .y=bitmap->height};
        dispbuf_draw_bitmap(cursor, bitmap_size, bitmap_size, bitmap->data, 0);
    }
    return bitmap->width;
}

int dispbuf_draw_label(DISPLAY_COORD cursor, const char*s, const FONT_TABLE* font, DRAW_FLAGS flags) {
    int cur_offset = 0;

    while (*s != 0) {
        int char_width = dispbuf_draw_character(cursor, *s, font, flags);
        cur_offset += char_width;
        cursor.x += char_width;
        s++;
    }

    return cur_offset;
}

int dispbuf_draw_label_with_string_size(DISPLAY_COORD cursor,
                             const char*s, size_t len,
                             const FONT_TABLE* font,
                             DRAW_FLAGS flags) {
    int cur_offset = 0;
    while (len--) {
        int char_width = dispbuf_draw_character(cursor, *s, font, flags);
        cur_offset += char_width;
        cursor.x += char_width;
        s++;
    }
    return cur_offset;
}

int DISPBUF_DrawWord(DISPLAY_COORD cursor,
                     const char *s,
                     const FONT_TABLE* font,
                     DRAW_FLAGS flags) {
    int cur_offset = 0;
    while (!DISPBUF_IsWhitespace(*s) && *s != '\0') {
        int char_width = dispbuf_draw_character(cursor, *s, font, flags);
        cur_offset += char_width;
        cursor.x += char_width;
        s++;
    }
    return cur_offset;
}

DISPLAY_COORD dispbuf_draw_text(DISPLAY_COORD cursor, const char *s, const FONT_TABLE* font,
                          uint16_t max_x, uint16_t max_y, DRAW_FLAGS flags) {

    // TODO: need to reduce complexity and potentially break down this function.

    int y_offset = 0;
    // Recursive solution is hacky here...
    if (flags & (DRAW_JUSTIFY_VERT_CENTER | DRAW_JUSTIFY_VERT_BOTTOM)) {
        DRAW_FLAGS measure_flags = (flags & ~(DRAW_JUSTIFY_VERT_CENTER | DRAW_JUSTIFY_VERT_BOTTOM)) | DRAW_MEASURE;
        int y_size = dispbuf_draw_text(cursor, s, font, max_x, max_y, measure_flags).y;

        if (flags & DRAW_JUSTIFY_VERT_CENTER) {
            y_offset = (max_y-y_size)/2;
        } else { // Bottom justify
            y_offset = max_y-y_size;
        }
    }

    cursor.y += y_offset;

    int max_width = 0;

    int chars_drawn = 0;
    int lines_drawn = 0;
    int font_height = DISPBUF_FontHeight(font);
    DISPLAY_COORD current_cursor = {
        .x = cursor.x,
        .y = cursor.y,
    };

    while (*s != 0) {
        // Measure and draw lines

        if (max_y < font_height) {
            // Can't fit any more lines
            break;
        }

        int line_pix_length = 0;
        const char* line_draw_start = s;
        const char* line_draw_end = s;
        const char* line_end = s; // Can include extra whitespace
        while (1) {
            // line rendering
            if (DISPBUF_IsNewline(*line_end)) {
                line_end++;
                break;
            } else if (*line_end == '\0') {
                break;
            } else if (DISPBUF_IsWhitespace(*line_end)) {
                // Safe to draw whitespace as measure only in all scenarios
                line_pix_length += dispbuf_draw_character(current_cursor, *line_end, font, flags | DRAW_MEASURE);
                line_end++;
                continue;
            } else {
                int word_pix_length = DISPBUF_DrawWord(current_cursor, line_end, font, flags | DRAW_MEASURE);
                if (word_pix_length + line_pix_length <= max_x) {
                    // Can draw this word!
                    line_pix_length += word_pix_length;
                    while ((*line_end != '\0') && !DISPBUF_IsWhitespace(*line_end)) {
                        line_end++;
                    }
                    line_draw_end = line_end;
                } else {
                    // real big word. Need to apply wrapping so we can continue!
                    if (word_pix_length > max_x) {
                        while (line_pix_length < max_x) {
                            int char_len = dispbuf_draw_character(current_cursor, *line_end, font, flags | DRAW_MEASURE);
                            if (char_len + line_pix_length < max_x) {
                                line_pix_length += char_len;
                                line_end++;
                                line_draw_end = line_end;
                            } else {
                                break;
                            }
                        }
                    }
                    break;
                }
            }

        }

        if (flags & DRAW_JUSTIFY_HORIZ_CENTER) {
            current_cursor.x += (max_x - line_pix_length) / 2;
        } else if (flags & DRAW_JUSTIFY_HORIZ_RIGHT) {
            current_cursor.x += (max_x - line_pix_length);
        }

        int width = dispbuf_draw_label_with_string_size(current_cursor, line_draw_start, line_draw_end-line_draw_start, font, flags);
        if (max_width < width) {
            max_width = width;
        }

        chars_drawn += line_end - line_draw_start;
        lines_drawn += 1;
        line_draw_start = line_end;
        line_draw_end = line_end;
        s = line_end;

        current_cursor.y += font_height;
        current_cursor.x = cursor.x;
        max_y -= font_height;
    }

    DISPLAY_COORD size = {
        .x = max_width,
        .y = lines_drawn * DISPBUF_FontHeight(font),
    };
    return size;
}
