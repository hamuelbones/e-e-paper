//
// Created by Samuel Jones on 1/9/22.
//

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/conf.h>
#include <string.h>

void CRYPTOGRAPHY_Init(void) {
    OPENSSL_add_all_algorithms_noconf();
}

static unsigned char _public_key[2048];
static unsigned char _private_key[2048];

void CRYPTOGRAPHY_RSA_Generate() {
    EVP_PKEY *key = EVP_RSA_gen(2048);

    BIO *public_out = BIO_new(BIO_s_mem());
    BIO *private_out = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(public_out, key);
    PEM_write_bio_PrivateKey_traditional(private_out, key, NULL, NULL, 0, NULL, NULL);

    BUF_MEM *public_buf = NULL;
    BUF_MEM *private_buf = NULL;
    BIO_get_mem_ptr(public_out, &public_buf);
    BIO_get_mem_ptr(private_out, &private_buf);

    // These now hold PEM encoded RSA keys!
    memcpy(_public_key, public_buf->data, public_buf->length);
    memcpy(_private_key, private_buf->data, private_buf->length);

    EVP_PKEY_free(key);

}

void CRYPTOGRAPHY_RSA_Sign() {
#if 0
    // Copied from an example!!!

    EVP_PKEY_CTX *ctx;
    /* md is a SHA-256 digest in this example. */
    unsigned char *md, *sig;
    size_t mdlen = 32, siglen;
    EVP_PKEY *signing_key;

    /*
     * NB: assumes signing_key and md are set up before the next
     * step. signing_key must be an RSA private key and md must
     * point to the SHA-256 digest to be signed.
     */
    ctx = EVP_PKEY_CTX_new(signing_key, NULL /* no engine */);
    if (!ctx)
        /* Error occurred */
        if (EVP_PKEY_sign_init(ctx) <= 0)
            /* Error */
            if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)
                /* Error */
                if (EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256()) <= 0)
                    /* Error */

                    /* Determine buffer length */
                    if (EVP_PKEY_sign(ctx, NULL, &siglen, md, mdlen) <= 0)
                        /* Error */

                        sig = OPENSSL_malloc(siglen);

    if (!sig)
        /* malloc failure */

        if (EVP_PKEY_sign(ctx, sig, &siglen, md, mdlen) <= 0) {}
    /* Error */

    /* Signature is siglen bytes written to buffer sig */
#endif
}

void CRYPTOGRAPHY_RSA_Verify() {

}

void CRYPTOGRAPHY_RSA_HasKey() {

}

void CRYPTOGRAPHY_SHA256_Digest() {

}