//
// Created by Samuel Jones on 1/9/22.
//

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/conf.h>
#include <openssl/rand.h>
#include <string.h>
#include "filesystem_hal.h"
#include "FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"

//
// Created by Samuel Jones on 1/9/22.
//

#include "cryptography_hal.h"


static unsigned char _public_key[2048];
static unsigned char _private_key[2048];

void cryptography_init(void) {
    OPENSSL_add_all_algorithms_noconf();
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

// Generate a new set of RSA keys on the filesystem.
bool cryptography_rsa_generate(const char *private_filename,
                               const char *public_filename,
                               const char *uuid_filename) {


    FS_Remove(uuid_filename);
    void* f = FS_Open(uuid_filename, "wb");

    if (!f) {
        return false;
    }

    char uuid[100] = {'\0'};
    uint8_t bytes[16] = {0};
    uint8_t *b = bytes;

    RAND_bytes(bytes, 16);
    int len = snprintf(uuid, 100, "uuid = \"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\"\n",
                       b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
    FS_Write(f, uuid, len);
    FS_Close(f);

    EVP_PKEY *key = EVP_RSA_gen(2048);

    BIO *public_out = BIO_new(BIO_s_mem());
    BIO *private_out = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(public_out, key);
    PEM_write_bio_PrivateKey_traditional(private_out, key, NULL, NULL, 0, NULL, NULL);

    BUF_MEM *public_buf = NULL;
    BUF_MEM *private_buf = NULL;
    BIO_get_mem_ptr(public_out, &public_buf);
    BIO_get_mem_ptr(private_out, &private_buf);


    FS_Remove(private_filename);
    f = FS_Open(private_filename, "wb");
    if (!f) {
        BIO_free(private_out);
        BIO_free(public_out);
        EVP_PKEY_free(key);
        return false;
    }
    FS_Write(f, private_buf->data, (int) private_buf->length);
    FS_Close(f);

    FS_Remove(public_filename);
    f = FS_Open(public_filename, "wb");
    if (!f) {
        FS_Remove(private_filename);
        BIO_free(private_out);
        BIO_free(public_out);
        EVP_PKEY_free(key);
        return false;
    }
    FS_Write(f, public_buf->data, (int) public_buf->length);

    BIO_free(private_out);
    BIO_free(public_out);
    EVP_PKEY_free(key);

    return true;
}

bool cryptography_digest_sha(const uint8_t *data,
                             size_t len,
                             size_t bits,
                             uint8_t *output) {

    const EVP_MD *digest = NULL;
    if (bits == 256) {
        // Assumed 32 byte output
        digest = EVP_sha256();
    } else if (bits == 384) {
        // 48 byte output
        digest = EVP_sha384();
    } else if (bits == 512) {
        // 64 byte output
        digest = EVP_sha512();
    } else {
        return false;
    }

    EVP_MD_CTX * md_ctx = EVP_MD_CTX_new();
    EVP_DigestInit(md_ctx, digest);
    EVP_DigestUpdate(md_ctx, data, len);
    EVP_DigestFinal(md_ctx, output, NULL);

    return true;
}

bool cryptography_sign_rsa(const char* private_filename,
                           const uint8_t *data,
                           size_t len,
                           uint8_t **output,
                           size_t *output_len) {

    // TODO support other RSA bit/digest lengths
    if (len != 32) {
        return false;
    }

    void* f = FS_Open(private_filename, "r");
    if (!f) {
        return false;
    }
    FS_Read(f, _private_key, 2048);
    FS_Close(f);

    BIO *key_bio = BIO_new_mem_buf(_private_key, -1);

    EVP_PKEY *key =  NULL;
    PEM_read_bio_PrivateKey(key_bio, &key, NULL, NULL);
    BIO_free(key_bio);

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(key, NULL /* no engine */);
    EVP_PKEY_sign_init(ctx);
    EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING);
    EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256());

    // To get signature length
    EVP_PKEY_sign(ctx, NULL, output_len, data, len);

    if (*output_len == 0) {
        printf("Sig generation failed\n");
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    *output = pvPortMalloc(*output_len);

    int retval = EVP_PKEY_sign(ctx, *output, output_len, data, len);
    EVP_PKEY_free(key);
    EVP_PKEY_CTX_free(ctx);

    return retval == 1;
}
