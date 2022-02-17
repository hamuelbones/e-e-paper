//
// Created by Samuel Jones on 1/20/22.
//
// base64 - encoding of binary data to the standard base64 format

#ifndef EPAPER_DISPLAY_BASE64_H
#define EPAPER_DISPLAY_BASE64_H

#include <stdint.h>
#include <stddef.h>

// Encodes base64 data. In order to fit the output, the output array should be at least
// ceil(len * 4/3) in size.
// Returns actual size of the encoded data.
size_t base64_encode(uint8_t* output, size_t len, const uint8_t *input);

#endif //EPAPER_DISPLAY_BASE64_H
