//
// Created by Samuel Jones on 11/26/21.
//

#include "wifi_task.h"
#include "wifi_hal.h"
#include "cryptography_hal.h"
#include "jwt.h"
#include <printf.h>


typedef enum {
    WIFI_STATE_INIT,

    WIFI_STATE_CONNECT,


};


static TaskHandle_t _wifiHandle;

bool _generate_jwt_signature_rsa(void* params, const uint8_t *message, size_t len, uint8_t** signature, size_t *sig_len) {
    uint8_t sha_result[32];
    cryptography_digest_sha(message, len, 256, sha_result);
    return cryptography_sign_rsa("private.pem", sha_result, 32, signature, sig_len);
}

static char cur_jwt[600];
static MessageBufferHandle_t _WifiMessageBuffer;

#define WIFI_MESSAGE_SIZE (20)

void _Noreturn wifi_task(void* params) {

#if 1
    vTaskDelay(10);

    // Test JWT!
    JWT_CTX *jwt = jwt_new("RS256", 1642824154, 1000, _generate_jwt_signature_rsa, NULL);
    jwt_add_string(jwt, JWT_HEADER, "kid", "6F8D5846-FA83-46F7-8007-60C0414A5518");
    strcpy(cur_jwt, "Authorization: Bearer ");
    size_t offset = strlen(cur_jwt);
    size_t token_size = jwt_serialize(jwt, cur_jwt+offset, 600-offset);
    printf("Token size: %zu\n", token_size);
    printf("%s\n", cur_jwt);

    jwt_destroy(jwt);
#endif

    _WifiMessageBuffer = xMessageBufferCreate(50);

    WIFI_Init(_WifiMessageBuffer);
    WIFI_Connect("ham", "smokyradio");

    char * headers[] = {
            cur_jwt
    };

    WIFI_HttpGet("127.0.0.1:8000",
                 "/dev_api/",
                 (const char **) headers,
                 1);

    while (1) {

        uint8_t inbound_message[WIFI_MESSAGE_SIZE] = {0};
        xMessageBufferReceive(_WifiMessageBuffer, inbound_message, WIFI_MESSAGE_SIZE, portMAX_DELAY);

    }
}

TaskHandle_t wifi_task_handle(void) {
    return _wifiHandle;
}


void wifi_task_start(void) {
    xTaskCreate(wifi_task,
                "WIFI",
                4096,
                NULL,
                5,
                &_wifiHandle);
}