//
// Created by Samuel Jones on 1/20/22.
//

#ifndef EPAPER_DISPLAY_BASE64_H
#define EPAPER_DISPLAY_BASE64_H

#include <stdint.h>
#include <stddef.h>


// Output should be at least ceil(len * 4/3) in size
size_t base64_encode(uint8_t* output, size_t len, const uint8_t *input);

#endif //EPAPER_DISPLAY_BASE64_H
