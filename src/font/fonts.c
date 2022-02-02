//
// Created by Samuel Jones on 12/20/21.
//

#include "fonts.h"

#include "headers/system_symbols.h"
#include "headers/bitter-ht-10.h"
#include "headers/bitter-ht-14.h"
#include "headers/bitter-ht-16.h"
#include "headers/bitter-ht-20.h"
#include "headers/bitter-ht-24.h"
#include "headers/bitter-ht-32.h"
#include "headers/bitter-ht-40.h"
#include "headers/bitter-ht-48.h"
#include <string.h>

const FONT_TABLE* fonts[FONT_MAX] = {
        [SYSTEM_SYMBOLS] = &font_System_Symbols,
        [BITTER_PRO_10] = &font_Bitter_Pro_Regular_10,
        [BITTER_PRO_14] = &font_Bitter_Pro_Regular_14,
        [BITTER_PRO_16] = &font_Bitter_Pro_Regular_16,
        [BITTER_PRO_20] = &font_Bitter_Pro_Regular_20,
        [BITTER_PRO_24] = &font_Bitter_Pro_Regular_24,
        [BITTER_PRO_32] = &font_Bitter_Pro_Regular_32,
        [BITTER_PRO_40] = &font_Bitter_Pro_Regular_40,
        [BITTER_PRO_48] = &font_Bitter_Pro_Regular_48,
};

const char* font_names[FONT_MAX] = {
        "SYSTEM_SYMBOLS",
        "BITTER_PRO_10",
        "BITTER_PRO_14",
        "BITTER_PRO_16",
        "BITTER_PRO_20",
        "BITTER_PRO_24",
        "BITTER_PRO_32",
        "BITTER_PRO_40",
        "BITTER_PRO_48",
};

const FONT_TABLE *FONT_GetTable(EPAPER_DISPLAY_FONT_ID id) {
    return fonts[id];
}

const FONT_CHARACTER *FONT_GetBitmap(const FONT_TABLE *font, int c) {

    if ((c < font->code_base) || (c >= font->code_base+font->code_length)) {
        return NULL;
    }

    return font->characters[c - font->code_base];
}


const char* FONT_GetName(EPAPER_DISPLAY_FONT_ID id) {
    if (id >= FONT_MAX) {
        return NULL;
    }
    return font_names[id];
}

const FONT_TABLE *FONT_TableForName(const char* name) {

    for (int i=0; i<FONT_MAX; i++) {
        const char *font_name = FONT_GetName(i);
        if (strcmp(font_name, name) == 0) {
            return FONT_GetTable(i);
        }
    }

    return NULL;
}