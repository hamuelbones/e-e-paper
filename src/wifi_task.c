//
// Created by Samuel Jones on 11/26/21.
//

#include "wifi_task.h"
#include "wifi_hal.h"
#include "toml_resources.h"
#include "message_buffer.h"

#define WIFI_MESSAGE_BUFFER_SIZE 200
#define WIFI_MESSAGE_MAX_SIZE 50

static MessageBufferHandle_t message_buffer;
static TaskHandle_t _wifiHandle;

void _Noreturn wifi_task(void* params) {


    message_buffer = xMessageBufferCreate(WIFI_MESSAGE_BUFFER_SIZE);
    WIFI_Init();

    while (1) {
        WIFI_REQUEST request = {0};
        size_t rx_size = xMessageBufferReceive(message_buffer, &request, sizeof(WIFI_REQUEST), portMAX_DELAY);

        if (rx_size <= 0) {
            printf("WiFi: No message received\n");
            continue;
        }

        switch (request.type) {
            case WIFI_CONNECT: {
                toml_table_t *startup = toml_resource_get("startup");
                toml_array_t* wifi_credentials = toml_array_in(startup, "wifi");
                int num_wifi_aps = toml_array_nelem(wifi_credentials);
                bool succeeded = false;
                for (int i=0; i<num_wifi_aps; i++) {
                    toml_table_t *current_creds = toml_table_at(wifi_credentials, i);

                    toml_datum_t ssid = toml_string_in(current_creds, "ssid");
                    toml_datum_t password = toml_string_in(current_creds, "password");

                    if (!ssid.ok) {
                        continue;
                    }

                    if (WIFI_Connect(ssid.u.s, password.ok ? password.u.s : NULL)) {
                        // success
                        succeeded = true;
                        break;
                    }
                }
                if (request.cb) {
                    WIFI_CONNECT_RESPONSE resp_data = {.connected = succeeded};
                    request.cb(request.cb_params, &resp_data);
                }
                break;
            }
            case WIFI_DISCONNECT:
                WIFI_Disconnect();
                if (request.cb) {
                    request.cb(request.cb_params, NULL);
                }
                break;
            case WIFI_GET: {
                int status = 0;
                bool succeeded = WIFI_HttpGet(request.get.host,
                                              request.get.subdirectory,
                                              (const char**)request.get.headers,
                                              request.get.header_count,
                                              request.get.headers_filename,
                                              request.get.response_filename,
                                              &status);
                if (request.cb) {
                    WIFI_GET_RESPONSE response = {.status = status};
                    request.cb(request.cb_params, &response);
                }
            }
                break;
            case WIFI_SYNC_TIME: {
                uint32_t time = WIFI_GetNetworkTime(request.sync_time.url);
                if (request.cb) {
                    WIFI_SYNC_TIME_RESPONSE response = {.unix_time = time};
                    request.cb(request.cb_params, &response);
                }
            }
                break;
        }

    }
}


void wifi_task_start(void) {
    xTaskCreate(wifi_task,
                "WIFI",
                4096,
                NULL,
                5,
                &_wifiHandle);
}

MessageBufferHandle_t wifi_message_buffer(void) {
    return message_buffer;
}