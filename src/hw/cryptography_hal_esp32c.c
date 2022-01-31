//
// Created by Samuel Jones on 1/9/22.
//

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "cryptography_hal.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"
#include "mbedtls/pk.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "filesystem_hal.h"
#include "esp_task_wdt.h"

void cryptography_init(void) {
}

int _cryptography_fill_random(void* ctx, unsigned char *data, size_t len) {
    esp_fill_random(data, len);
    return 0;
}


bool cryptography_rsa_exists(const char* private_filename,
                             const char* public_filename,
                             const char* uuid_filename) {
    struct stat s;
    // TODO do proper validation and backup/restore
    if (0 == FS_Stat(private_filename, &s) && 0 == FS_Stat(public_filename, &s)) {
        return true;
    }
    return false;
}

// Generate a new set of RSA keys on the filesystem. Generate UUID to use for the key set.
bool cryptography_rsa_generate(const char *private_filename,
                               const char *public_filename,
                               const char *uuid_filename) {

    printf("%s\n", uuid_filename);
    FS_Remove(uuid_filename);
    file_handle f = FS_Open(uuid_filename, "wb");
    if (!f) {
        printf("Failed to open key UUID file\n");
        return false;
    }

    unsigned char bytes[16];
    uint8_t *b = bytes;
    esp_fill_random(bytes, 16);

    char uuid[100];
    int len = snprintf(uuid, 100, "uuid = \"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\"\n",
                       b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
    FS_Write(f, uuid, len);
    FS_Close(f);

    mbedtls_pk_context pk_ctx = {0};
    mbedtls_pk_init(&pk_ctx);
    int result = mbedtls_pk_setup( &pk_ctx, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA) );
    if (result != 0) {
        printf("pk_setup failed: %i\n", result);
    }

    esp_task_wdt_init(30, false);
    result = mbedtls_rsa_gen_key(mbedtls_pk_rsa(pk_ctx), _cryptography_fill_random, NULL, 2048, 0x10001);
    if (result != 0) {
        mbedtls_pk_free(&pk_ctx);
        printf("RSA Generation error: %d\n", result);
        return false;
    }
    result = mbedtls_rsa_check_pub_priv(mbedtls_pk_rsa(pk_ctx), mbedtls_pk_rsa(pk_ctx));
    if (result != 0) {
        mbedtls_pk_free(&pk_ctx);
        printf("RSA validation error: %d\n", result);
        return false;
    }

    unsigned char *key_data = pvPortMalloc(5000);

    FS_Remove(private_filename);
    f = FS_Open(private_filename, "wb");
    if (!f) {
        printf("Failed to open private key file\n");
        mbedtls_pk_free(&pk_ctx);
        vPortFree(key_data);
        return false;
    }

    result = mbedtls_pk_write_key_pem(&pk_ctx, key_data, 5000);
    if (result != 0) {
        printf("Error exporting private key\n");
        FS_Close(f);
        mbedtls_pk_free(&pk_ctx);
        vPortFree(key_data);
        return false;
    }

    // TODO check file write errors;
    FS_Write(f, key_data, (int)strlen((char*)key_data));
    FS_Close(f);

    FS_Remove(public_filename);
    f = FS_Open(public_filename, "wb");
    if (!f) {
        printf("Failed to open public key file\n");
        mbedtls_pk_free(&pk_ctx);
        vPortFree(key_data);
        return false;
    }

    result = mbedtls_pk_write_pubkey_pem(&pk_ctx, key_data, 5000);
    if (result != 0) {
        printf("Error exporting public key\n");
        FS_Close(f);
        mbedtls_pk_free(&pk_ctx);
        vPortFree(key_data);
        return false;
    }

    FS_Write(f, key_data, (int)strlen((char*)key_data));
    FS_Close(f);

    vPortFree(key_data);
    mbedtls_pk_free(&pk_ctx);

    return true;
}


bool cryptography_digest_sha(const uint8_t *data,
                             size_t len,
                             size_t bits,
                             uint8_t *output) {

    if (bits != 256) {
        printf("Unsupported SHA mode\n");
        return false;
    }
    int result = mbedtls_sha256_ret(data, len, output, 0);

    if (result != 0) {
        printf("SHA hash calculation failure! (%d)", result);
        return false;
    }
    return true;
}

bool cryptography_sign_rsa(const char* private_filename,
                           const uint8_t *data,
                           size_t len,
                           uint8_t **output,
                           size_t *output_len) {

    mbedtls_pk_context pk_ctx = {0};
    mbedtls_pk_init(&pk_ctx);
    //mbedtls_pk_setup( &pk_ctx, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));

    struct stat fstat;
    int error = FS_Stat(private_filename, &fstat);

    if (error) {
        printf("No PEM file for signing!\n");
        mbedtls_pk_free(&pk_ctx);
        return false;
    }

    int result;
    result = mbedtls_pk_parse_keyfile(&pk_ctx, private_filename, NULL);
    if (result != 0) {
        printf("Failed to import data from PEM file! (%d)\n", result);
        return false;
    }

    *output = pvPortMalloc(256);

    result = mbedtls_pk_sign(&pk_ctx, MBEDTLS_MD_SHA256, data, len, *output, output_len, _cryptography_fill_random, NULL);
    mbedtls_pk_free(&pk_ctx);
    if (result != 0) {
        printf("Failed to generate signature! (%d)\n", result);
        vPortFree(output);
        return false;
    }

    return true;
}