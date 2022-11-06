//
// Created by Samuel Jones on 1/10/22.
//

#include "jwt.h"
#include "freertos/FreeRTOS.h"
#include <stdio.h>
#include "base64.h"

#define JWT_DEFAULT_SIZE (100u)

#define JWT_OPEN_SECTION "\{"
#define JWT_END_SECTION "\}"


void jwt_append_token(JWT_ENTRY *entry, const char* token, size_t len) {
    bool resize = false;
    while (entry->size + len >= entry->max_size) {
        entry->max_size *= 2;
        resize = true;
    }

    if (resize) {
        char * old_buffer = entry->data;
        char * new_buffer = pvPortMalloc(entry->max_size);
        memcpy(new_buffer, old_buffer, entry->size);
        entry->data = new_buffer;
        memset(old_buffer, 0, entry->size);
        vPortFree(old_buffer);
    }

    memcpy(&entry->data[entry->size], token, len);
    entry->size += len;
}

void jwt_append_string(JWT_ENTRY *entry, const char* string) {
    size_t len = strlen(string);
    jwt_append_token(entry, string, len);
}

void jwt_start_kv(JWT_ENTRY *entry) {
    if (entry->size) {
        // Add a comma
        jwt_append_token(entry, ",", 1);
    } else {
        // Add opening bracket
        jwt_append_token(entry, "{", 1);
    }
}

void jwt_encode_string(JWT_ENTRY *entry, const char* string) {
    jwt_append_token(entry, "\"", 1);
    jwt_append_string(entry, string);
    jwt_append_token(entry, "\"", 1);
}

void jwt_encode_uint(JWT_ENTRY *entry, uint32_t uint) {
    char encoded_integer[12];
    size_t len = snprintf(encoded_integer, 12, "%u", (unsigned int) uint);
    jwt_append_token(entry, encoded_integer, len);
}


JWT_CTX *jwt_new(const char* algorithm, uint32_t unix_time, uint32_t duration,
                 bool(*gen_sig)(void*, const uint8_t*, size_t, uint8_t**, size_t*), void* sig_params) {
    JWT_CTX *jwt = pvPortMalloc(sizeof(JWT_CTX));
    jwt->header.data = pvPortMalloc(JWT_DEFAULT_SIZE);
    jwt->header.size = 0;
    jwt->header.max_size = JWT_DEFAULT_SIZE;
    jwt->claims.data = pvPortMalloc(JWT_DEFAULT_SIZE);
    jwt->claims.size = 0;
    jwt->claims.max_size = JWT_DEFAULT_SIZE;
    jwt->gen_signature = gen_sig;
    jwt->sig_params = sig_params;

    jwt_add_string(jwt, JWT_HEADER, "typ", "JWT");
    jwt_add_string(jwt, JWT_HEADER, "alg", algorithm);
    jwt_add_uint(jwt, JWT_PAYLOAD, "iat", unix_time);
    jwt_add_uint(jwt, JWT_PAYLOAD, "exp", unix_time+duration);

    return jwt;
}

void jwt_add_string(JWT_CTX *jwt, JWT_LOCATION location, const char *key, const char* value) {
    JWT_ENTRY *e;
    if (location == JWT_HEADER) {
        e = &jwt->header;
    } else if (location == JWT_PAYLOAD) {
        e = &jwt->claims;
    } else {
        return;
    }

    jwt_start_kv(e);
    jwt_encode_string(e, key);
    jwt_append_token(e, ":", 1);
    jwt_encode_string(e, value);
}

void jwt_add_uint(JWT_CTX *jwt, JWT_LOCATION location, const char *key, uint32_t value) {

    JWT_ENTRY *e;
    if (location == JWT_HEADER) {
        e = &jwt->header;
    } else if (location == JWT_PAYLOAD) {
        e = &jwt->claims;
    } else {
        return;
    }

    jwt_start_kv(e);
    jwt_encode_string(e, key);
    jwt_append_token(e, ":", 1);
    jwt_encode_uint(e, value);
}

size_t jwt_serialize(JWT_CTX *jwt, char *output, size_t max_len) {
    jwt_append_token(&jwt->header, "}", 1);
    jwt_append_token(&jwt->claims, "}", 1);

    size_t b64_header_size = ((jwt->header.size + 2) / 3) * 4;
    size_t b64_claims_size = ((jwt->claims.size + 2) / 3) * 4;
    size_t size_to_digest = b64_header_size + 1 + b64_claims_size;

    unsigned char *b64_data = pvPortMalloc(size_to_digest);
    base64_encode(b64_data, jwt->header.size, (const unsigned char*) jwt->header.data);
    for (int i=0; i<2; i++) {
        // Need to remove padding!
        if (b64_data[b64_header_size - 1] == '=') {
            b64_header_size--;
            size_to_digest--;
        } else {
            break;
        }
    }
    b64_data[b64_header_size] = '.';
    base64_encode(&b64_data[b64_header_size + 1], jwt->claims.size, (const unsigned char*) jwt->claims.data);
    for (int i=0; i<2; i++) {
        // Need to remove padding!
        if (b64_data[b64_header_size + 1 + b64_claims_size - 1] == '=') {
            b64_claims_size--;
            size_to_digest--;
        } else {
            break;
        }
    }

    size_t signature_size = 0;
    uint8_t *signature = NULL;
    bool successful = jwt->gen_signature(jwt->sig_params, b64_data, size_to_digest, &signature, &signature_size);
    if (!successful) {
        vPortFree(b64_data);
        return 0;
    }

    size_t b64_signature_size = ((signature_size + 2) / 3) * 4;
    size_t token_size = size_to_digest + 1 + b64_signature_size;
    if (token_size + 1 >= max_len) {
        printf("token size too big");
        vPortFree(b64_data);
        vPortFree(signature);
        return 0;
    }

    memcpy(output, b64_data, size_to_digest);
    output[size_to_digest] = '.';
    base64_encode((unsigned char*)&output[size_to_digest+1], signature_size, signature);
    for (int i=0; i<2; i++) {
        // Need to remove padding!
        if (output[token_size - 1] == '=') {
            token_size--;
        } else {
            break;
        }
    }

    vPortFree(b64_data);
    vPortFree(signature);
    output[token_size] = '\0';

    return token_size;
}

void jwt_destroy(JWT_CTX *jwt) {
    vPortFree(jwt->claims.data);
    vPortFree(jwt->header.data);
    if (jwt->sig_params) {
        vPortFree(jwt->sig_params);
    }
    vPortFree(jwt);
}