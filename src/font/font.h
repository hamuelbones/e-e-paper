//
// Created by Samuel Jones on 12/19/21.
//

#ifndef EPAPER_DISPLAY_FONT_H
#define EPAPER_DISPLAY_FONT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    const uint8_t *data;
    const uint8_t width;
    const uint8_t height;
} FONT_CHARACTER;

typedef struct {
    uint16_t code_base;
    uint16_t code_length;
    const FONT_CHARACTER** characters;
} FONT_TABLE;

#endif //EPAPER_DISPLAY_FONT_H
