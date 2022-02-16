//
// Created by Samuel Jones on 12/20/21.
//

#ifndef EPAPER_DISPLAY_FONTS_H
#define EPAPER_DISPLAY_FONTS_H

#include "font.h"

typedef enum {
    SYSTEM_SYMBOLS = 0,
    NEW_YORK_10,
    NEW_YORK_12,
    NEW_YORK_14,
    NEW_YORK_18,
    NEW_YORK_20,
    NEW_YORK_24,
    NEW_YORK_36,
    FONT_MAX,
} EPAPER_DISPLAY_FONT_ID;

const FONT_TABLE *font_get_table(EPAPER_DISPLAY_FONT_ID id);
const FONT_CHARACTER *font_get_bitmap(const FONT_TABLE *font, int c);
const char* font_get_name(EPAPER_DISPLAY_FONT_ID id);
const FONT_TABLE *font_get_table_for_name(const char* name);

#endif //EPAPER_DISPLAY_FONTS_H
