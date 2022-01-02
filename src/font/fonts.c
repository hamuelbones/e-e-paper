//
// Created by Samuel Jones on 12/20/21.
//

#include "fonts.h"

#include "headers/bitter-ht-10.h"
#include "headers/bitter-ht-14.h"
#include "headers/bitter-ht-16.h"
#include "headers/bitter-ht-20.h"
#include "headers/bitter-ht-24.h"
#include "headers/bitter-ht-32.h"
#include "headers/bitter-ht-40.h"
#include "headers/bitter-ht-48.h"

const FONT_TABLE* fonts[] = {
        [BITTER_PRO_10] = &font_Bitter_Pro_Regular_10,
        [BITTER_PRO_14] = &font_Bitter_Pro_Regular_14,
        [BITTER_PRO_16] = &font_Bitter_Pro_Regular_16,
        [BITTER_PRO_20] = &font_Bitter_Pro_Regular_20,
        [BITTER_PRO_24] = &font_Bitter_Pro_Regular_24,
        [BITTER_PRO_32] = &font_Bitter_Pro_Regular_32,
        [BITTER_PRO_40] = &font_Bitter_Pro_Regular_40,
        [BITTER_PRO_48] = &font_Bitter_Pro_Regular_48,
};

const FONT_CHARACTER *FONT_GetBitmap(EPAPER_DISPLAY_FONT_ID id, int c) {

    const FONT_TABLE *font = fonts[id];
    if ((c < font->code_base) || (c >= font->code_base+font->code_length)) {
        return NULL;
    }

    return font->characters[c];
}
