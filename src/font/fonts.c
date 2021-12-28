//
// Created by Samuel Jones on 12/20/21.
//

#include "fonts.h"

#include "font_helvetica_14.h"

const FONT_TABLE* fonts[] = {
        [HELVETICA_14] = &font_Helvetica_14,
};

const FONT_CHARACTER *FONT_GetBitmap(EPAPER_DISPLAY_FONT_ID id, int c) {

    const FONT_TABLE *font = fonts[id];
    if ((c < font->code_base) || (c >= font->code_base+font->code_length)) {
        return NULL;
    }

    return font->characters[c];
}
