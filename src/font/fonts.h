//
// Created by Samuel Jones on 12/20/21.
//

#ifndef EPAPER_DISPLAY_FONTS_H
#define EPAPER_DISPLAY_FONTS_H

#include "font.h"

typedef enum {
    SYSTEM_SYMBOLS = 0,
    BITTER_PRO_10,
    BITTER_PRO_14,
    BITTER_PRO_16,
    BITTER_PRO_20,
    BITTER_PRO_24,
    BITTER_PRO_32,
    BITTER_PRO_40,
    BITTER_PRO_48,
    FONT_MAX,
} EPAPER_DISPLAY_FONT_ID;

const FONT_CHARACTER *FONT_GetBitmap(EPAPER_DISPLAY_FONT_ID id, int c);
const char* FONT_GetName(EPAPER_DISPLAY_FONT_ID id);

#endif //EPAPER_DISPLAY_FONTS_H
