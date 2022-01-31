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

// TODO TIME hal!
uint32_t last_unix_time = 0;
uint32_t current_resource_fetch_id = 0;

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
    MAIN_MESSAGE_CONFIG_OR_RESOURCE_READY,
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

static char jwt_header[600];

bool _generate_jwt_signature_rsa(void* params, const uint8_t *message, size_t len, uint8_t** signature, size_t *sig_len) {
    uint8_t sha_result[32];
    cryptography_digest_sha(message, len, 256, sha_result);
    return cryptography_sign_rsa(INTERNAL_MOUNT_POINT "private.pem", sha_result, 32, signature, sig_len);
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

    uint8_t message[5] = {MAIN_MESSAGE_TIME_SYNCED};
    if (!sync_time->unix_time) {
        message[0] = MAIN_MESSAGE_TIME_SYNC_FAILED;
    } else {
        memcpy(&message[1], &sync_time->unix_time, 4);
    }
    xMessageBufferSend(message_buffer, &message, 5, portMAX_DELAY);
}

static void _handle_get(void* params, void* response) {
    WIFI_GET_RESPONSE *get = response;

    uint8_t message[3] = {MAIN_MESSAGE_CONFIG_OR_RESOURCE_READY, get->status & 0xFF, get->status>>8 & 0xFF};
    xMessageBufferSend(message_buffer, &message, 3, portMAX_DELAY);

    WIFI_GET_ARGS *args = params;
    vPortFree(args->headers);
}

static void _issue_get_request(const char* host, const char* subdirectory, const char* destination, bool use_jwt) {

    WIFI_REQUEST request = {
            .type = WIFI_GET,
            .cb = _handle_get,
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
        char* headers[1] = {pvPortMalloc(sizeof(char*))};
        headers[0] = jwt_header;

        request.get.headers = headers;
        request.get.header_count = 1;
    }

    // Provide context struct back so we can clean up
    request.cb_params = &request.get;

    xMessageBufferSend(wifi_message_buffer(), &request, sizeof(WIFI_REQUEST), portMAX_DELAY);
}

static bool _refresh_resource(int num) {

    printf("Attempting to get resource %u\n", num);

    toml_array_t* resources = toml_array_in(toml_resource_get("config"), "resource");
    if (num >= toml_array_nelem(resources)) {
        printf("no more resources\n");
        return false;
    }

    toml_table_t* resource_info = toml_table_at(resources, num);
    if (!resource_info) {
        return false;
    }

    toml_datum_t name = toml_string_in(resource_info, "local_filename");
    toml_datum_t host = toml_string_in(resource_info, "host");
    toml_datum_t dir = toml_string_in(resource_info, "dir");
    toml_datum_t auth = toml_bool_in(resource_info, "auth");

    _issue_get_request(host.u.s, dir.u.s, SD_MOUNT_POINT REQUEST_TEMPORARY_FILENAME, auth.u.b);

    return true;
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
        uint8_t message = MAIN_MESSAGE_CONFIG_OR_RESOURCE_READY;
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

    toml_array_t *resources = toml_array_in(device_config, "resource");
    if (resources) {
        current_resource_fetch_id = 0;
        _refresh_resource(current_resource_fetch_id);

        printf("Fetching resources specified in configuration file\n");
        return MAIN_STATE_REFRESH_RESOURCES;
    }

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

            memcpy(&last_unix_time, &message[1], 4);

            char uuid[37] = "invalid key";
            file_load_uuid(SD_MOUNT_POINT "key_uuid.toml", uuid);
            // Generate JWT now that we ahve network time
            JWT_CTX *jwt = jwt_new("RS256", last_unix_time, 1000, _generate_jwt_signature_rsa, NULL);
            jwt_add_string(jwt, JWT_HEADER, "kid", uuid);
            strcpy(jwt_header, "Authorization: Bearer ");
            size_t offset = strlen(jwt_header);
            size_t token_size = jwt_serialize(jwt, jwt_header+offset, 600-offset);

            jwt_destroy(jwt);

            toml_table_t *startup = toml_resource_get("startup");
            toml_table_t *server = toml_table_in(startup, "server");
            if (!server) {
                printf("No server configuration found.\n");
                return MAIN_STATE_INIT_ERROR;
            }
            toml_datum_t host, subdirectory;
            host = toml_string_in(server, "host");
            if (!host.ok) {
                printf("No server hostname provided for configuration.\n");
                return MAIN_STATE_INIT_ERROR;
            }
            subdirectory = toml_string_in(server, "config_dir");
            if (!subdirectory.ok) {
                printf("No server subdirectory provided for configuration.\n");
            }

            _issue_get_request(host.u.s, subdirectory.u.s, SD_MOUNT_POINT REQUEST_TEMPORARY_FILENAME, true);

            return MAIN_STATE_REFRESH_CONFIG;
        }
        case MAIN_MESSAGE_TIME_SYNC_FAILED:
            return MAIN_STATE_SYNC_TIME;
    }
    return -1;
}

static int _state_refresh_config(uint8_t *message, size_t len) {
    switch(message[0]) {
        case MAIN_MESSAGE_CONFIG_OR_RESOURCE_READY: {
            int16_t status = -1;
            if (!standalone && len > 2) {
                status = message[1] | (message[2] << 8);
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

    switch(message[0]) {
        case MAIN_MESSAGE_CONFIG_OR_RESOURCE_READY: {
            int16_t status = message[1] | (message[2] << 8);

            toml_array_t* resources = toml_array_in(toml_resource_get("config"), "resource");
            toml_table_t* resource_info = toml_table_at(resources, (int)current_resource_fetch_id);
            toml_datum_t name = toml_string_in(resource_info, "local_filename");;
            toml_datum_t resource_name = toml_string_in(resource_info, "name");

            if (!name.ok) {
                printf("Resource name not specified...\n");
                current_resource_fetch_id++;
                if (_refresh_resource(current_resource_fetch_id)) {
                    return -1;
                } else {
                    return MAIN_STATE_RUN_APP;
                }
            }

            char from[60];
            char to[60];
            snprintf(from, 60, "%s%s", SD_MOUNT_POINT, REQUEST_TEMPORARY_FILENAME);
            snprintf(to, 60, "%s%s", SD_MOUNT_POINT, name.u.s);
            if (status == 200) {
                // Move file to correct location on SD
                FS_Remove(to);
                FS_Rename(from, to);
            }

            strcpy(from, to);
            snprintf(to, 60, "%s%s", INTERNAL_MOUNT_POINT, name.u.s);

            // Move file from SD to internal
            if (file_exists(from)) {
                file_copy(from, to);
            }

            toml_resource_load(to, resource_name.u.s);

            current_resource_fetch_id++;
            if (_refresh_resource(current_resource_fetch_id)) {
                return -1;
            } else {
                return MAIN_STATE_RUN_APP;
            }
        }
        default:
            break;
    }
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


void _Noreturn app_main(void)
{
    uint8_t inbound_message[MAIN_MESSAGE_MAX_SIZE] = {0};
    MAIN_STATE current_state = MAIN_STATE_INIT;

#if 0
    while (1) {
        vTaskDelay(100);
    }
#endif

    HAL_Init();
    FS_Mount();

    // TODO What if there is not an SD card? need to use internal only
    cryptography_init();
    if (!cryptography_rsa_exists(INTERNAL_MOUNT_POINT "private.pem", SD_MOUNT_POINT "public.pem", SD_MOUNT_POINT "key_uuid.toml")) {
        cryptography_rsa_generate(INTERNAL_MOUNT_POINT "private.pem",  SD_MOUNT_POINT "public.pem", SD_MOUNT_POINT "key_uuid.toml");
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
