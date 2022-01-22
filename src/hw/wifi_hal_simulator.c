//
// Created by Samuel Jones on 1/21/22.
//

#include "wifi_hal.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <unistd.h>


static MessageBufferHandle_t _message_buffer;
static SSL_CTX *ssl;
static uint8_t _response_data[4001];
static size_t _response_size;

void WIFI_Init(MessageBufferHandle_t message_buffer) {
    _message_buffer = message_buffer;

    SSL_library_init();
    SSL_load_error_strings();
    ssl = SSL_CTX_new(TLS_client_method());
}

void WIFI_Connect(const char* ssid, const char* password) {

    char message = WIFI_HAL_CONNECTED;
    xMessageBufferSend(_message_buffer, &message, 1, portMAX_DELAY);

}

void WIFI_Disconnect() {

    char message = WIFI_HAL_DISCONNECTED;
    xMessageBufferSend(_message_buffer, &message, 1, portMAX_DELAY);
}

WIFI_HTTP_REQ_ID WIFI_HttpGet(const char* host,
                              const char* subdirectory,
                              const char** headers,
                              size_t header_count) {

    BIO * bio = BIO_new_ssl_connect(ssl);
    SSL * thisSSL;
    BIO_get_ssl(bio, &thisSSL);
    SSL_set_mode(thisSSL, SSL_MODE_AUTO_RETRY);

    BIO_set_conn_hostname(bio, host);

    int result = 0;
    do {
        result = BIO_do_connect(bio);
    } while (result <= 0 && BIO_should_retry(bio));

    if (result <= 0) {
        printf("Failed connection %d\n", result);
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        return INVALID_HTTP_REQ_ID;
    }

    if(BIO_do_handshake(bio) <= 0) {
        fprintf(stderr, "Error establishing SSL connection\n");
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        return INVALID_HTTP_REQ_ID;
    }

    char line[200] = {0};
    int len = 0;
    len = snprintf(line, 200, "GET %s HTTP/1.1\r\n", subdirectory);
    BIO_write(bio, line, len);
    len = snprintf(line, 200, "Host: %s\r\n", host);
    BIO_write(bio, line, len);
    BIO_puts(bio, "Connection: close\r\n");
    for (size_t i = 0; i<header_count; i++) {
        BIO_puts(bio, headers[i]);
        BIO_puts(bio, "\r\n");
    }
    BIO_puts(bio, "\r\n");
    BIO_flush(bio);



    _response_size = 0;
    int max_stall_count = 1000;
    int stall_count = 0;
    int us_per_count = 1000;
    while (1) {
        int size = BIO_read(bio, &_response_data[_response_size], 4000-_response_size);

        if (size > 0) {
            _response_size += size;
            stall_count = 0;
        } else {
            stall_count++;
            if (stall_count >= max_stall_count) {
                break;
            }
            usleep(us_per_count);
            continue;
        }

        if (_response_size >= 4000) {
            break;
        }
    }
    _response_data[_response_size] = '\0';
    printf("%s\n\n", _response_data);

    BIO_free_all(bio);
}