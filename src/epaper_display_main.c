/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/message_buffer.h"
#include "freertos/timers.h"

#include "wifi_task.h"
#include "display_task.h"
#include "filesystem_hal.h"
#include "toml.h"
#include "cryptography_hal.h"
#include "file_utils.h"
#include "toml_resources.h"
#include "cryptography_hal.h"
#include "jwt.h"
#include "epaper_display_main.h"

#include "apps.h"
#include "message_box_app.h"

#include "init_hal.h"

#define NUMBER_OF_APPS (1)

#define MAIN_MESSAGE_BUFFER_SIZE (400)
#define MAIN_MESSAGE_MAX_SIZE (100)


#define AUTHOR_SUBSTITUTION_VALUE "!!MSG_BOX_AUTHOR!!"
#define MESSAGE_SUBSTITUTION_VALUE "!!MSG_BOX_MESSAGE!!"


typedef enum {
    // Check startup configuration
    MAIN_STATE_INIT,
    // TODO display something here
    MAIN_STATE_INIT_ERROR,
    // Safe mode - don't load anything. TODO Go here if we're in some sort of bootloop
    MAIN_STATE_SAFE,
    // WiFi Connection
    MAIN_STATE_CONNECT,
    // Get time
    MAIN_STATE_SYNC_TIME,
    // Getting config.toml from the server
    MAIN_STATE_REFRESH_CONFIG,
    // Getting resources described in the config.toml
    MAIN_STATE_REFRESH_RESOURCES,
    MAIN_STATE_RUN_APP,
    MAIN_STATE_MAX,
} MAIN_STATE;

typedef enum {
    MAIN_MESSAGE_LOAD_STARTUP,
    MAIN_MESSAGE_WIFI_CONNECTED,
    MAIN_MESSAGE_WIFI_CONNECT_FAILED,
    MAIN_MESSAGE_TIME_SYNCED,
    MAIN_MESSAGE_TIME_SYNC_FAILED,
    MAIN_MESSAGE_CONFIG_READY,
    MAIN_MESSAGE_RESOURCE_READY,
    MAIN_MESSAGE_APP_INIT,
    MAIN_MESSAGE_APP,
} MAIN_STATE_MESSAGE;

typedef enum {
    MAIN_APP_TIMER_PROC,
    MAIN_APP_CUSTOM_ID_START
} MAIN_APP_MESSAGE;

static MessageBufferHandle_t message_buffer;

static const APP_INTERFACE *_apps[NUMBER_OF_APPS] = {
    &g_message_box_interface
};

static void* _currentAppContext = NULL;
static xTimerHandle _currentAppTimer = NULL;
static bool standalone = false;

static char cur_jwt[600];

bool _generate_jwt_signature_rsa(void* params, const uint8_t *message, size_t len, uint8_t** signature, size_t *sig_len) {
    uint8_t sha_result[32];
    cryptography_digest_sha(message, len, 256, sha_result);
    return cryptography_sign_rsa("private.pem", sha_result, 32, signature, sig_len);
}

static void _app_timer_callback(TimerHandle_t timer) {
    uint8_t message[2] = {MAIN_MESSAGE_APP, MAIN_APP_TIMER_PROC};
    xMessageBufferSend(message_buffer, &message, 2, 0);
}

static void _handle_connection(void* params, void* response) {
    WIFI_CONNECT_RESPONSE *connect = response;
    uint8_t message[1] = {MAIN_MESSAGE_WIFI_CONNECTED};
    if (!connect->connected) {
        message[0] = MAIN_MESSAGE_WIFI_CONNECT_FAILED;
    }
    xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
}

static void _handle_time_synced(void* params, void* response) {
    WIFI_SYNC_TIME_RESPONSE *sync_time = response;

    uint8_t message[1] = {MAIN_MESSAGE_TIME_SYNCED};
    if (!sync_time->unix_time) {
        message[0] = MAIN_MESSAGE_TIME_SYNC_FAILED;
    }
    xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
}

static void _handle_config_refresh(void* params, void* response) {
    WIFI_GET_RESPONSE *get = response;

    uint8_t message[3] = {MAIN_MESSAGE_CONFIG_READY, get->status & 0xFF, get->status>>8 & 0xFF};
    xMessageBufferSend(message_buffer, &message, 3, portMAX_DELAY);

    // Nothing dynamically allocated, nothing to clean up.
}


static void _issue_get_request(const char* host, const char* subdirectory, const char* destination, bool use_jwt) {

    WIFI_REQUEST request = {
            .type = WIFI_GET,
            .cb = _handle_config_refresh,
            .get = {
                    .host = (char*)host,
                    .subdirectory = (char*)subdirectory,
                    .headers = NULL,
                    .header_count = 0,
                    .headers_filename = NULL,
                    .response_filename = (char*) destination,
            },
    };

    if (use_jwt) {
        request.get.headers = (char **) &cur_jwt;
        request.get.header_count = 1;
    }

    // Provide context struct back so we can clean up
    request.cb_params = &request.get;
}


static int _load_startup_file(void) {

    // save settings From SD if it exists.
    if (file_exists(SD_MOUNT_POINT STARTUP_FILENAME)) {
        file_copy(INTERNAL_MOUNT_POINT STARTUP_FILENAME, SD_MOUNT_POINT STARTUP_FILENAME);
    }

    if (!file_exists(INTERNAL_MOUNT_POINT STARTUP_FILENAME)) {
        printf("No TOML file in internal storage!\n");
        return MAIN_STATE_INIT_ERROR;
    }

    if (!toml_resource_load(INTERNAL_MOUNT_POINT STARTUP_FILENAME, "startup")) {
        printf("Failed to load startup TOML\n");
        return MAIN_STATE_INIT_ERROR;
    }

    toml_table_t *startup_config = toml_resource_get("startup");
    if (!startup_config) {
        printf("Failed to load startup TOML from resource list\n");
        return MAIN_STATE_INIT_ERROR;
    }

    toml_datum_t mode = toml_string_in(startup_config, "mode");
    if (!mode.ok) {
        printf("Mode unspecified\n");
        return MAIN_STATE_INIT_ERROR;
    }

    printf("Toml startup file loaded!\n");
    if (strcmp("standalone", mode.u.s) == 0) {
        standalone = true;
        // Standalone mode - we should expect config to already exist!
        uint8_t message = MAIN_MESSAGE_CONFIG_READY;
        xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);

        return MAIN_STATE_REFRESH_CONFIG;
    } else if (strcmp("online", mode.u.s) == 0) {
        // Online mode - we should connect to WiFi
        standalone = false;
        WIFI_REQUEST request = {
            .type = WIFI_CONNECT,
            .cb = _handle_connection,
        };
        xMessageBufferSend(wifi_message_buffer(), &request, sizeof(request), portMAX_DELAY);


        return MAIN_STATE_CONNECT;
    } else {
        printf("Unknown run mode\n");
        return MAIN_STATE_INIT_ERROR;
    }

}

static int _load_config_file(void) {

    toml_resource_load(INTERNAL_MOUNT_POINT APP_CONFIG_FILENAME, "config");
    toml_table_t *device_config = toml_resource_get("config");
    if (!device_config) {
        printf("Toml config file load error\n");
        return MAIN_STATE_INIT_ERROR;
    }

    printf("TOML configuration file loaded \n");

    uint8_t message = MAIN_MESSAGE_APP_INIT;
    xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
    return MAIN_STATE_RUN_APP;
}

static int _state_init(uint8_t *message, size_t len)  {
    switch (message[0]) {
        case MAIN_MESSAGE_LOAD_STARTUP:
            return _load_startup_file();
        default:
            break;
    }

    return -1;
}

static int _state_connect(uint8_t *message, size_t len) {
    switch(message[0]) {
        case MAIN_MESSAGE_WIFI_CONNECTED: {

            // Generate JWT in preparation for connection
            JWT_CTX *jwt = jwt_new("RS256", 1642824154, 1000, _generate_jwt_signature_rsa, NULL);
            jwt_add_string(jwt, JWT_HEADER, "kid", "6F8D5846-FA83-46F7-8007-60C0414A5518");
            strcpy(cur_jwt, "Authorization: Bearer ");
            size_t offset = strlen(cur_jwt);
            size_t token_size = jwt_serialize(jwt, cur_jwt+offset, 600-offset);
            printf("Token size: %zu\n", token_size);
            printf("%s\n", cur_jwt);

            jwt_destroy(jwt);

            // Now sync time
            WIFI_REQUEST request = {
                    .type = WIFI_SYNC_TIME,
                    .cb = _handle_time_synced,
                    .sync_time.url = "pool.ntp.org",
            };

            xMessageBufferSend(wifi_message_buffer(), &request, sizeof(request), portMAX_DELAY);
        }
            // NEXT THING: Send message to wifi task!
            return MAIN_STATE_SYNC_TIME;
        case MAIN_MESSAGE_WIFI_CONNECT_FAILED:
            return MAIN_STATE_INIT_ERROR;
    }
    return -1;
}


static int _state_refresh_time(uint8_t *message, size_t len) {
    switch(message[0]) {
        case MAIN_MESSAGE_TIME_SYNCED: {

            WIFI_REQUEST request = {
                    .type = WIFI_GET,
                    .cb = _handle_config_refresh,
                    .get = {
                            .host = "",
                            .subdirectory = "",
                            .headers = NULL,
                            .header_count = 0,
                            .headers_filename = NULL,
                            .response_filename = NULL,
                    },
            };

            xMessageBufferSend(wifi_message_buffer(), &request, sizeof(request), portMAX_DELAY);
        }
            return MAIN_STATE_REFRESH_CONFIG;
        case MAIN_MESSAGE_TIME_SYNC_FAILED:
            return MAIN_STATE_SYNC_TIME;
    }
    return -1;
}

static int _state_refresh_config(uint8_t *message, size_t len) {
    switch(message[0]) {
        case MAIN_MESSAGE_CONFIG_READY: {
            int16_t status = -1;
            if (!standalone && len >= 2) {
                status = message[0] | (message[1] << 8);
                printf("Config refresh status: %u\n", status);
            }

            if (status == 200) {
                // Move file to correct location on SD
                FS_Remove(SD_MOUNT_POINT APP_CONFIG_FILENAME);
                FS_Rename(SD_MOUNT_POINT REQUEST_TEMPORARY_FILENAME, SD_MOUNT_POINT APP_CONFIG_FILENAME);
            }

            // Move file from SD to internal
            if (file_exists(SD_MOUNT_POINT APP_CONFIG_FILENAME)) {
                file_copy(INTERNAL_MOUNT_POINT APP_CONFIG_FILENAME, SD_MOUNT_POINT APP_CONFIG_FILENAME);
            }

            // May load resources if necessary
            int next_state = _load_config_file();
            if (standalone && (next_state == MAIN_STATE_REFRESH_RESOURCES)) {
                // Should not hit network in standalone mode!
                next_state = MAIN_STATE_RUN_APP;
            }

            return next_state;
        }
        default:
            break;
    }
    return -1;
}

static int _state_refresh_resources(uint8_t *message, size_t len) {
    return -1;
}

static int _state_run_app(uint8_t *message, size_t len) {

    static const APP_INTERFACE *current_app;

    switch(message[0]) {
        case MAIN_MESSAGE_APP_INIT: {
            current_app = NULL;
            toml_table_t *device_config = toml_resource_get("config");
            toml_table_t *app_info = toml_table_in(device_config, "application");
            toml_datum_t app_name = toml_string_in(app_info, "name");

            toml_table_t *startup_config = toml_resource_get("startup");

            for (int i=0; i<NUMBER_OF_APPS; i++) {
                if (strcmp(app_name.u.s, _apps[i]->name) == 0) {
                    current_app = _apps[i];
                    break;
                }
            }
            if (current_app) {
                _currentAppContext = current_app->app_init(startup_config, device_config);
                if (!_currentAppTimer) {
                    _currentAppTimer = xTimerCreate("App", current_app->refresh_rate_ms*1000/configTICK_RATE_HZ, pdTRUE,
                                                    NULL, _app_timer_callback);
                } else {
                    xTimerChangePeriod(_currentAppTimer, current_app->refresh_rate_ms*1000/configTICK_RATE_HZ, portMAX_DELAY);
                }
                xTimerStart(_currentAppTimer, portMAX_DELAY);
            }
        }
            break;
        case MAIN_MESSAGE_APP: {
            if (current_app) {
                current_app->app_process(_currentAppContext, message+1, len-1);
            }
        }
        default:
            break;
    }
    return -1;
}

static int (*_main_states[MAIN_STATE_MAX])(uint8_t * message, size_t len) = {
        [MAIN_STATE_INIT] = _state_init,
        // Init Error
        // Safe Mode
        [MAIN_STATE_CONNECT] = _state_connect,
        [MAIN_STATE_SYNC_TIME] = _state_refresh_time,
        [MAIN_STATE_REFRESH_CONFIG] = _state_refresh_config,
        [MAIN_STATE_REFRESH_RESOURCES] = _state_refresh_resources,
        [MAIN_STATE_RUN_APP] = _state_run_app,
};

static size_t toml_fs_read(void* ptr, size_t size, size_t nitems, void* stream) {
    return FS_Read(stream, ptr, size*nitems);
}

void _Noreturn app_main(void)
{
    uint8_t inbound_message[MAIN_MESSAGE_MAX_SIZE] = {0};
    MAIN_STATE current_state = MAIN_STATE_INIT;

    HAL_Init();
    FS_Mount();

    cryptography_init();
    if (!cryptography_rsa_exists("private.pem", "public.pem")) {
        cryptography_rsa_generate("private.pem", "public.pem");
    }

    toml_set_memutil(pvPortMalloc, vPortFree);
    toml_set_futil( FS_Feof, toml_fs_read);

    message_buffer = xMessageBufferCreate(MAIN_MESSAGE_BUFFER_SIZE);
    uint8_t startMessage = MAIN_MESSAGE_LOAD_STARTUP;
    xMessageBufferSend(message_buffer, &startMessage, 1, portMAX_DELAY);

    wifi_task_start();
    display_task_start();

    while (1) {
        size_t rx_size = xMessageBufferReceive(message_buffer, inbound_message, MAIN_MESSAGE_MAX_SIZE, portMAX_DELAY);

        if (rx_size <= 0) {
            printf("Main: No message received\n");
            continue;
        }
        if (_main_states[current_state]) {
            int next_state = _main_states[current_state](inbound_message, rx_size);
            if (next_state >= 0 && next_state < MAIN_STATE_MAX) {
                current_state = next_state;
            }
        }
    }
}


MessageBufferHandle_t MAIN_GetMessageBuffer(void) {
    return message_buffer;
}
