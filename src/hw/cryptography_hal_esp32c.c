//
// Created by Samuel Jones on 1/9/22.
//

#include <stdint.h>
#include <stddef.h>
#include "cryptography_hal.h"
#include "mbedtls/rsa.h"
#include "mbedtls/pem.h"

void cryptography_init(void) {

}


bool cryptography_rsa_exists(const char* private_filename,
                             const char* public_filename,
                             const char* uuid_filename) {
    return false;
}

// Generate a new set of RSA keys on the filesystem. Generate UUID to use for the key set.
bool cryptography_rsa_generate(const char *private_filename,
                               const char *public_filename,
                               const char *uuid_filename) {
    return false;
}


bool cryptography_digest_sha(const uint8_t *data,
                             size_t len,
                             size_t bits,
                             uint8_t *output) {
    return false;
}

bool cryptography_sign_rsa(const char* private_filename,
                           const uint8_t *data,
                           size_t len,
                           uint8_t **output,
                           size_t *output_len) {
    return false;
}