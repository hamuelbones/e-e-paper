//
// Created by Samuel Jones on 1/20/22.
//

#include "base64.h"
#include <string.h>
#include <stdio.h>

uint8_t raw_to_character(uint8_t input) {
    if (input <= 25) {
        return input + 'A';
    } else if (input <= 51) {
        return input + 'a' - 26;
    } else if (input <= 61) {
        return input + '0' - 52;
    } else if (input == 62) {
        // Use URL-safe encoding, "+" normally
        return '-';
    } else if (input == 63) {
        // Use URL-safe encoding, "_" normally
        return '_';
    }

    return 0;
}

size_t base64_encode(uint8_t* output, size_t len, const uint8_t *input) {
    size_t count = 0;
    printf("b64_encode: %.*s\n", (int)len, input);

    while (len) {
        uint32_t chunk_size = len >= 3 ? 3 : len;
        uint8_t encoded_chunk[4] = {'=', '=', '=', '='};

        encoded_chunk[0] = input[0] >> 2;
        encoded_chunk[1] = (input[0] << 4) & 0x3F;

        if (chunk_size >= 2) {
            encoded_chunk[1] |= input[1] >> 4;
            encoded_chunk[2] = (input[1] << 2) & 0x3F;
        }

        if (chunk_size >= 3) {
            encoded_chunk[2] |= input[2] >> 6;
            encoded_chunk[3] = input[2] & 0x3F;
        }
        for (int i=0; i<chunk_size+1; i++) {
            encoded_chunk[i] = raw_to_character(encoded_chunk[i]);
        }

        memcpy(output, encoded_chunk, 4);

        len -= chunk_size;
        input += chunk_size;
        count += 4;
        output += 4;
    }

    return count;
}