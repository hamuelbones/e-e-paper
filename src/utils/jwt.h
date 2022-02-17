//
// Created by Samuel Jones on 1/10/22.
//
// jwt - utilities for creating tokens JSON web tokens for authorization

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef EPAPER_DISPLAY_JWT_H
#define EPAPER_DISPLAY_JWT_H

#define MAX_KEYS 3

typedef struct {
    char * data;
    size_t size;
    size_t max_size;
} JWT_ENTRY;

typedef struct {
    JWT_ENTRY header;
    JWT_ENTRY claims;
    bool (*gen_signature)(void* params, const uint8_t *input, size_t len, uint8_t **sig, size_t* sig_len);
    void* sig_params;
} JWT_CTX;

typedef enum {
    JWT_HEADER,
    JWT_PAYLOAD
} JWT_LOCATION;

JWT_CTX *jwt_new(const char* algorithm, uint32_t unix_time, uint32_t duration,
                 bool(*gen_signature)(void*, const uint8_t*, size_t, uint8_t**, size_t*), void* sig_params);
void jwt_add_string(JWT_CTX *jwt, JWT_LOCATION location, const char *key, const char* value);
void jwt_add_uint(JWT_CTX *jwt, JWT_LOCATION location, const char *key, uint32_t value);
size_t jwt_serialize(JWT_CTX *jwt, char *output, size_t max_len);
void jwt_destroy(JWT_CTX *jwt);


#endif //EPAPER_DISPLAY_JWT_H
