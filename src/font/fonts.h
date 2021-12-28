//
// Created by Samuel Jones on 12/20/21.
//

#ifndef EPAPER_DISPLAY_FONTS_H
#define EPAPER_DISPLAY_FONTS_H

#include "font.h"

typedef enum {
    HELVETICA_14 = 0,
} EPAPER_DISPLAY_FONT_ID;

const FONT_CHARACTER *FONT_GetBitmap(EPAPER_DISPLAY_FONT_ID id, int c);

#endif //EPAPER_DISPLAY_FONTS_H
