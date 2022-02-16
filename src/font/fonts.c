//
// Created by Samuel Jones on 12/20/21.
//

#include "fonts.h"

#include "headers/system_symbols.h"
#include "headers/new-york-10.h"
#include "headers/new-york-12.h"
#include "headers/new-york-14.h"
#include "headers/new-york-18.h"
#include "headers/new-york-20.h"
#include "headers/new-york-24.h"
#include "headers/new-york-36.h"
#include <string.h>

const FONT_TABLE* fonts[FONT_MAX] = {
        [SYSTEM_SYMBOLS] = &font_System_Symbols,
        [NEW_YORK_10] = &font_New_York_10,
        [NEW_YORK_12] = &font_New_York_12,
        [NEW_YORK_14] = &font_New_York_14,
        [NEW_YORK_18] = &font_New_York_18,
        [NEW_YORK_20] = &font_New_York_20,
        [NEW_YORK_24] = &font_New_York_24,
        [NEW_YORK_36] = &font_New_York_36,
};

const char* font_names[FONT_MAX] = {
        "SYSTEM_SYMBOLS",
        "NEW_YORK_10",
        "NEW_YORK_12",
        "NEW_YORK_14",
        "NEW_YORK_18",
        "NEW_YORK_20",
        "NEW_YORK_24",
        "NEW_YORK_36",
};

const FONT_TABLE *font_get_table(EPAPER_DISPLAY_FONT_ID id) {
    return fonts[id];
}

const FONT_CHARACTER *font_get_bitmap(const FONT_TABLE *font, int c) {

    if ((c < font->code_base) || (c >= font->code_base+font->code_length)) {
        return NULL;
    }

    return font->characters[c - font->code_base];
}


const char* font_get_name(EPAPER_DISPLAY_FONT_ID id) {
    if (id >= FONT_MAX) {
        return NULL;
    }
    return font_names[id];
}

const FONT_TABLE *font_get_table_for_name(const char* name) {

    for (int i=0; i<FONT_MAX; i++) {
        const char *font_name = font_get_name(i);
        if (strcmp(font_name, name) == 0) {
            return font_get_table(i);
        }
    }

    return NULL;
}