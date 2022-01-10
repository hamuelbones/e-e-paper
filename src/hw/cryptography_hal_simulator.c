//
// Created by Samuel Jones on 1/9/22.
//

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/conf.h>

void CRYPTOGRAPHY_Init(void) {
    OPENSSL_add_all_algorithms_noconf();
}

void CRYPTOGRAPHY_RSA_Generate() {
}