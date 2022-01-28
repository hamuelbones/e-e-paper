//
// Created by Samuel Jones on 1/9/22.
//

#include <stdbool.h>

#ifndef EPAPER_DISPLAY_CRYPTOGRAPHY_H
#define EPAPER_DISPLAY_CRYPTOGRAPHY_H

void cryptography_init(void);


bool cryptography_rsa_exists(const char* private_filename,
                             const char* public_filename,
                             const char* uuid_filename);

// Generate a new set of RSA keys on the filesystem. Generate UUID to use for the key set.
bool cryptography_rsa_generate(const char *private_filename,
                               const char *public_filename,
                               const char *uuid_filename);


bool cryptography_digest_sha(const uint8_t *data,
                             size_t len,
                             size_t bits,
                             uint8_t *output);

bool cryptography_sign_rsa(const char* private_filename,
                           const uint8_t *data,
                           size_t len,
                           uint8_t **output,
                           size_t *output_len);

#endif //EPAPER_DISPLAY_CRYPTOGRAPHY_H
