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

const FONT_TABLE *FONT_GetTable(EPAPER_DISPLAY_FONT_ID id);
const FONT_CHARACTER *FONT_GetBitmap(const FONT_TABLE *font, int c);
const char* FONT_GetName(EPAPER_DISPLAY_FONT_ID id);
const FONT_TABLE *FONT_TableForName(const char* name);

#endif //EPAPER_DISPLAY_FONTS_H
