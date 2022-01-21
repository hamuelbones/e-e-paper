//
// Created by Samuel Jones on 11/26/21.
//

#include "wifi_task.h"
#include "wifi_hal.h"
#include "cryptography_hal.h"
#include "jwt.h"
#include <printf.h>

static TaskHandle_t _wifiHandle;

bool _generate_jwt_signature_rsa(void* params, const uint8_t *message, size_t len, uint8_t** signature, size_t *sig_len) {
    uint8_t sha_result[32];
    cryptography_digest_sha(message, len, 256, sha_result);
    return cryptography_sign_rsa("private.pem", sha_result, 32, signature, sig_len);
}

static char cur_jwt[2000];
void _Noreturn wifi_task(void* params) {

    vTaskDelay(10);

    // Test JWT!
    JWT_CTX *jwt = jwt_new("RS256", 1500000000, 1000, _generate_jwt_signature_rsa, NULL);
    jwt_add_string(jwt, JWT_HEADER, "kid", "01234-5678");
    size_t token_size = jwt_serialize(jwt, cur_jwt, 2000);
    printf("Token size: %zu\n", token_size);
    printf("Token: %s\n", cur_jwt);

    jwt_destroy(jwt);

    WIFI_Init();
    WIFI_Connect("ham", "smokyradio");

    while (1) {
        vTaskDelay(1000 * 1000 / configTICK_RATE_HZ);
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