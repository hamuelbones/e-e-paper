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
#include "wifi_hal.h"
#include "filesystem_hal.h"
#include "toml.h"
#include "cryptography_hal.h"
#include "file_utils.h"
#include "resources.h"
#include "cryptography_hal.h"
#include "jwt.h"
#include "epaper_display_main.h"
#include "display_buffer.h"

#include "render_toml.h"
#include "epaper_hal.h"
#include "epapers/epapers.h"

#include "init_hal.h"

#define MAIN_MESSAGE_BUFFER_SIZE (400)
#define MAIN_MESSAGE_MAX_SIZE (100)

// TODO TIME hal!
uint32_t last_unix_time = 0;
uint32_t current_resource_fetch_id = 0;

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
    // Check for updates
    MAIN_STATE_UPDATE_CHECK,
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
    MAIN_MESSAGE_TRIGGER_REFRESH,
    MAIN_MESSAGE_APP,
    MAIN_MESSAGE_REBOOT,
} MAIN_STATE_MESSAGE;

typedef enum {
    MAIN_APP_TIMER_PROC,
    MAIN_APP_CUSTOM_ID_START
} MAIN_APP_MESSAGE;

static MessageBufferHandle_t message_buffer;



static TimerHandle_t _currentAppTimer = NULL;
static TimerHandle_t _refreshTimer = NULL;
static bool standalone = false;
static const EPAPER_SPI_HAL_CONFIG *_epaper = NULL;

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

static void _refresh_callback(TimerHandle_t timer) {
    uint8_t message[1] = {MAIN_MESSAGE_TRIGGER_REFRESH};
    xMessageBufferSend(message_buffer, &message, 1, 0);
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
    WIFI_REQUEST *request = params;

    uint8_t message[3] = {MAIN_MESSAGE_CONFIG_OR_RESOURCE_READY, get->status & 0xFF, get->status>>8 & 0xFF};
    xMessageBufferSend(message_buffer, &message, 3, portMAX_DELAY);

    if (request->get.headers) {
        vPortFree(request->get.headers);
    }
    vPortFree(request->get.host);
    vPortFree(request->get.subdirectory);
    vPortFree(request);
}

static void _issue_get_request(char* url, const char* destination, bool use_jwt, bool use_ssl, bool is_ota_update) {

    char* divider = strchr(url, '/');
    size_t host_len;
    char * host;
    char * directory;
    if (divider) {
        host_len = divider - url;
        host = pvPortMalloc(host_len+1);
        memcpy(host, url, host_len);
        host[host_len] = '\0';
        size_t directory_len = strlen(divider);
        directory = pvPortMalloc(directory_len+1);
        strcpy(directory, divider);
    } else {
        host_len = strlen(url);
        host = pvPortMalloc(host_len+1);
        strcpy(host, url);
        directory = pvPortMalloc(2);
        strcpy(directory, "/");
    }


    WIFI_REQUEST *request = pvPortMalloc(sizeof(WIFI_REQUEST));
    memset(request, 0, sizeof(WIFI_REQUEST));
    request->type = WIFI_GET;
    request->cb = _handle_get;
    request->get.host = (char*)host;
    request->get.subdirectory = (char*)directory;
    request->get.headers = NULL;
    request->get.use_ssl = use_ssl;
    request->get.is_ota_update = is_ota_update;
    request->get.header_count = 0;
    request->get.headers_filename = NULL;
    request->get.response_filename = (char*) destination;

    if (use_jwt) {
        char** headers = {pvPortMalloc(sizeof(char*))};
        headers[0] = jwt_header;

        request->get.headers = headers;
        request->get.header_count = 1;
    }

    // Provide context struct back so we can clean up
    request->cb_params = request;

    xMessageBufferSend(wifi_message_buffer(), request, sizeof(WIFI_REQUEST), portMAX_DELAY);
}

static bool _refresh_resource(int num) {

    printf("Attempting to get resource %u\n", num);

    TOML_RESOURCE_CONTEXT *toml_ctx = resource_get("config");
    toml_array_t* resources = toml_array_in(toml_ctx->document, "resource");
    if (num >= toml_array_nelem(resources)) {
        printf("no more resources\n");
        return false;
    }

    toml_table_t* resource_info = toml_table_at(resources, num);
    if (!resource_info) {
        return false;
    }

    toml_datum_t url = toml_string_in(resource_info, "url");
    toml_datum_t jwt = toml_bool_in(resource_info, "jwt");
    if (!jwt.ok) {
        jwt.u.b = false;
    }
    toml_datum_t ssl = toml_bool_in(resource_info, "ssl");
    if (!ssl.ok) {
        ssl.u.b = true;
    }

    if (standalone || (!url.ok)) {
        // Just load the resource
        uint8_t message = MAIN_MESSAGE_CONFIG_OR_RESOURCE_READY;
        xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
    } else {
        _issue_get_request(url.u.s, SD_MOUNT_POINT REQUEST_TEMPORARY_FILENAME, jwt.u.b, ssl.u.b, false);
    }

    if (url.ok) {
        vPortFree(url.u.s);
    }

    return true;
}


static int _load_startup_file(void) {

    resource_load("DUMMY", "system", RESOURCE_SYSTEM);
    resource_load("DUMMY", "time", RESOURCE_CLOCK);

    // save settings From SD if it exists.
    if (file_exists(SD_MOUNT_POINT STARTUP_FILENAME)) {
        file_copy(INTERNAL_MOUNT_POINT STARTUP_FILENAME, SD_MOUNT_POINT STARTUP_FILENAME);
    }

    if (!file_exists(INTERNAL_MOUNT_POINT STARTUP_FILENAME)) {
        printf("No TOML file in internal storage!\n");
        char message = MAIN_MESSAGE_REBOOT;
        xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
        return MAIN_STATE_INIT_ERROR;
    }

    if (!resource_load(INTERNAL_MOUNT_POINT STARTUP_FILENAME, "startup", RESOURCE_TOML)) {
        printf("Failed to load startup TOML\n");
        char message = MAIN_MESSAGE_REBOOT;
        xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
        return MAIN_STATE_INIT_ERROR;
    }

    TOML_RESOURCE_CONTEXT *toml_ctx = resource_get("startup");
    toml_table_t *startup_config = toml_ctx->document;
    if (!startup_config) {
        printf("Failed to load startup TOML from resource list\n");
        char message = MAIN_MESSAGE_REBOOT;
        xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
        return MAIN_STATE_INIT_ERROR;
    }

    toml_datum_t mode = toml_string_in(startup_config, "mode");
    if (!mode.ok) {
        printf("Mode unspecified\n");
        char message = MAIN_MESSAGE_REBOOT;
        xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
        return MAIN_STATE_INIT_ERROR;
    }

    int new_state = 0;
    printf("Toml startup file loaded!\n");
    if (strcmp("standalone", mode.u.s) == 0) {
        standalone = true;
        // Standalone mode - we should expect config to already exist!
        uint8_t message = MAIN_MESSAGE_CONFIG_OR_RESOURCE_READY;
        xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);

        new_state = MAIN_STATE_REFRESH_CONFIG;
    } else if (strcmp("online", mode.u.s) == 0) {
        // Online mode - we should connect to WiFi
        standalone = false;
        WIFI_REQUEST request = {
                .type = WIFI_CONNECT,
                .cb = _handle_connection,
        };
        xMessageBufferSend(wifi_message_buffer(), &request, sizeof(request), portMAX_DELAY);
        new_state = MAIN_STATE_CONNECT;
    } else {
        printf("Unknown run mode\n");
        char message = MAIN_MESSAGE_REBOOT;
        xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
        new_state = MAIN_STATE_INIT_ERROR;
    }

    vPortFree(mode.u.s);

    toml_datum_t display_name = toml_string_in(startup_config, "display");
    if (display_name.ok) {
        _epaper = epaper_get_config_for_name(display_name.u.s);
        if (!_epaper) {
            printf("No display matched: %s\n", display_name.u.s);
            new_state = MAIN_STATE_INIT_ERROR;
        }
        epaper_init(_epaper);
        dispbuf_init(_epaper->width, _epaper->height, _epaper->bw_inverted, _epaper->xy_flipped);
        vPortFree(display_name.u.s);
    }
    return new_state;

}

static int _load_config_file(bool update_check) {

    TOML_RESOURCE_CONTEXT *toml_ctx = resource_get("config");
    if (!toml_ctx) {
        resource_load(INTERNAL_MOUNT_POINT APP_CONFIG_FILENAME, "config", RESOURCE_TOML);
        toml_ctx = resource_get("config");
    }

    toml_table_t *device_config = toml_ctx->document;
    if (!device_config) {
        printf("Toml config file load error\n");
        char message = MAIN_MESSAGE_REBOOT;
        xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
        return MAIN_STATE_INIT_ERROR;
    }

    printf("TOML configuration file loaded \n");
    if (update_check) {
        toml_table_t *update_info = toml_table_in(device_config, "update");
        if (update_info) {
            toml_datum_t url = toml_string_in(update_info, "url");
            if (url.ok) {
                char *full_url = resource_make_substitutions(url.u.s);
                toml_datum_t jwt = toml_bool_in(update_info, "jwt");
                if (!jwt.ok) {
                    jwt.u.b = false;
                }
                toml_datum_t ssl = toml_bool_in(update_info, "ssl");
                if (!ssl.ok) {
                    ssl.u.b = false;
                }

                _issue_get_request(full_url, SD_MOUNT_POINT REQUEST_TEMPORARY_FILENAME, jwt.u.b, ssl.u.b, true);
                vPortFree(url.u.s);
                vPortFree(full_url);
                return MAIN_STATE_UPDATE_CHECK;
            }
        }

    }

    toml_array_t *resources = toml_array_in(device_config, "resource");
    if (resources) {
        current_resource_fetch_id = 0;
        _refresh_resource(current_resource_fetch_id);
        return MAIN_STATE_REFRESH_RESOURCES;
    }

    uint8_t message = MAIN_MESSAGE_APP_INIT;
    xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
    return MAIN_STATE_RUN_APP;
}

static int _state_init(uint8_t *message, size_t len)  {
    switch (message[0]) {
        case MAIN_MESSAGE_LOAD_STARTUP:
            printf("\nstarting up: app %s, ver %s\n\n", app_hal_name(), app_hal_version());
            return _load_startup_file();
        default:
            break;
    }

    return -1;
}

static int _state_init_failure(uint8_t* message, size_t len) {
    printf("Init error, rebooting in 20 seconds...");
    vTaskDelay(20000/portTICK_PERIOD_MS);
    app_hal_reboot();
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
        case MAIN_MESSAGE_WIFI_CONNECT_FAILED: {
            printf("Couldn't connect to WiFi");
            standalone = true;
            uint8_t message = MAIN_MESSAGE_CONFIG_OR_RESOURCE_READY;
            xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);

            return MAIN_STATE_REFRESH_CONFIG;
        }
    }
    return -1;
}

static int _attempt_to_get_config(void) {

    // Getting any response means that OTA update was not available or failed.

    char uuid[37] = "invalid key";
    file_load_uuid(SD_MOUNT_POINT "key_uuid.toml", uuid);
    // Generate JWT now that we ahve network time
    JWT_CTX *jwt = jwt_new("RS256", last_unix_time, 1000, _generate_jwt_signature_rsa, NULL);
    jwt_add_string(jwt, JWT_HEADER, "kid", uuid);
    strcpy(jwt_header, "Authorization: Bearer ");
    size_t offset = strlen(jwt_header);
    jwt_serialize(jwt, jwt_header+offset, 600-offset);

    jwt_destroy(jwt);

    TOML_RESOURCE_CONTEXT *toml_ctx = resource_get("startup");
    toml_table_t *startup = toml_ctx->document;
    toml_table_t *server = toml_table_in(startup, "server");
    if (!server) {
        printf("No server configuration found.\n");
        char message = MAIN_MESSAGE_REBOOT;
        xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
        return MAIN_STATE_INIT_ERROR;
    }
    toml_datum_t url;
    url = toml_string_in(server, "url");
    if (!url.ok) {
        printf("No server hostname provided for configuration.\n");
        char message = MAIN_MESSAGE_REBOOT;
        xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
        return MAIN_STATE_INIT_ERROR;
    }

    toml_datum_t ssl = toml_bool_in(server, "ssl");
    if (!ssl.ok) {
        ssl.u.b = true;
    }
    toml_datum_t use_jwt = toml_bool_in(server, "jwt");
    if (!use_jwt.ok) {
        use_jwt.u.b = true;
    }

    _issue_get_request(url.u.s, SD_MOUNT_POINT REQUEST_TEMPORARY_FILENAME, use_jwt.u.b, ssl.u.b, false);
    vPortFree(url.u.s);

    return MAIN_STATE_REFRESH_CONFIG;
}

static int _state_refresh_time(uint8_t *message, size_t len) {
    switch(message[0]) {
        case MAIN_MESSAGE_TIME_SYNCED: {
            memcpy(&last_unix_time, &message[1], 4);
            return _attempt_to_get_config();
        }
        case MAIN_MESSAGE_TIME_SYNC_FAILED:
            if (wifi_connected()) {
                WIFI_REQUEST request = {};
                request.type = WIFI_DISCONNECT;
                xMessageBufferSend(wifi_message_buffer(), &request, sizeof(WIFI_REQUEST), portMAX_DELAY);
            }
            resource_unload_all();
            return _load_startup_file();
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
                fs_remove(SD_MOUNT_POINT APP_CONFIG_FILENAME);
                fs_rename(SD_MOUNT_POINT REQUEST_TEMPORARY_FILENAME, SD_MOUNT_POINT APP_CONFIG_FILENAME);
            }

            // Move file from SD to internal
            if (file_exists(SD_MOUNT_POINT APP_CONFIG_FILENAME)) {
                file_copy(INTERNAL_MOUNT_POINT APP_CONFIG_FILENAME, SD_MOUNT_POINT APP_CONFIG_FILENAME);
            }

            // May load resources if necessary
            int next_state = _load_config_file(!standalone);
            return next_state;
        }
        default:
            break;
    }
    return -1;
}


static int _state_ota_update_check(uint8_t *message, size_t len) {

    if (message[0] == MAIN_MESSAGE_CONFIG_OR_RESOURCE_READY) {
        return _load_config_file(false);
    }
    return -1;
}


static int _state_refresh_resources(uint8_t *message, size_t len) {

    switch(message[0]) {
        case MAIN_MESSAGE_CONFIG_OR_RESOURCE_READY: {
            int16_t status = 0;
            if (len >= 3) {
                status = message[1] | (message[2] << 8);
            }

            TOML_RESOURCE_CONTEXT *ctx = resource_get("config");
            toml_array_t* resources = toml_array_in(ctx->document, "resource");
            toml_table_t* resource_info = toml_table_at(resources, (int)current_resource_fetch_id);
            toml_datum_t name = toml_string_in(resource_info, "local_filename");
            toml_datum_t resource_name = toml_string_in(resource_info, "name");

            if (!name.ok || !resource_name.ok) {
                if (name.ok) {
                    vPortFree(name.u.s);
                }
                if (resource_name.ok) {
                    vPortFree(resource_name.u.s);
                }

                printf("Resource file name not specified...\n");
                current_resource_fetch_id++;
                if (_refresh_resource(current_resource_fetch_id)) {
                    return -1;
                } else {
                    uint8_t message = MAIN_MESSAGE_APP_INIT;
                    xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
                    return MAIN_STATE_RUN_APP;
                }
            }

            char from[60];
            char to[60];
            snprintf(from, 60, "%s%s", SD_MOUNT_POINT, REQUEST_TEMPORARY_FILENAME);
            snprintf(to, 60, "%s%s", SD_MOUNT_POINT, name.u.s);
            if (status == 200) {
                // Move file to correct location on SD
                fs_remove(to);
                file_copy(to, from);
            }

            strcpy(from, to);
            snprintf(to, 60, "%s%s", INTERNAL_MOUNT_POINT, name.u.s);

            // Move file from SD to internal
            file_copy(to, from);

            // TODO support dots in filenames
            char * extension = strrchr(to, '.');
            if (extension) {
                if (strcmp(extension, ".toml") == 0) {
                    resource_load(to, resource_name.u.s, RESOURCE_TOML);
                } else if (strcmp(extension, ".fbin") == 0) {
                    resource_load(to, resource_name.u.s, RESOURCE_FONT);
                }
            }

            vPortFree(name.u.s);
            vPortFree(resource_name.u.s);

            current_resource_fetch_id++;
            if (_refresh_resource(current_resource_fetch_id)) {
                return -1;
            } else {
                uint8_t message = MAIN_MESSAGE_APP_INIT;
                xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);
                return MAIN_STATE_RUN_APP;
            }
        }
        default:
            break;
    }
    return -1;
}

static int _state_run_app(uint8_t *message, size_t len) {

    switch(message[0]) {
        case MAIN_MESSAGE_APP_INIT: {
            uint32_t refresh_seconds = 10;
            TOML_RESOURCE_CONTEXT *ctx = resource_get("config");
            toml_table_t *app_info = toml_table_in(ctx->document, "application");
            if (app_info) {
                toml_datum_t refresh = toml_int_in(app_info, "refresh_interval_sec");
                if (refresh.ok) {
                    refresh_seconds = refresh.u.i;
                }
            }

            // Done with WiFi!
            WIFI_REQUEST request = {};
            request.type = WIFI_DISCONNECT;
            xMessageBufferSend(wifi_message_buffer(), &request, sizeof(WIFI_REQUEST), portMAX_DELAY);

            if (!_currentAppTimer) {
                _currentAppTimer = xTimerCreate("App", 1000*refresh_seconds/portTICK_PERIOD_MS, pdTRUE,
                                                NULL, _app_timer_callback);
            } else {
                xTimerChangePeriod(_currentAppTimer, 1000*refresh_seconds/portTICK_PERIOD_MS, portMAX_DELAY);
            }
            xTimerStart(_currentAppTimer, portMAX_DELAY);

            if (!_refreshTimer) {
                _refreshTimer = xTimerCreate("Ref", 30*60*1000/portTICK_PERIOD_MS, pdTRUE, NULL, _refresh_callback);
            }
            _app_timer_callback(NULL);
        }
            break;
        case MAIN_MESSAGE_APP: {
            dispbuf_swap();
            dispbuf_clear_active();

            TOML_RESOURCE_CONTEXT *ctx = resource_get("config");
            toml_table_t *device_config = ctx->document;
            toml_table_t *drawing_root = toml_table_in(device_config, "render");
            DISPLAY_COORD dims = {epaper_config()->width, epaper_config()->height};
            render_toml(drawing_root, dims);

            epaper_render_buffer(_epaper, dispbuf_active_buffer(), dispbuf_inactive_buffer(), _epaper->width * _epaper->height/8);
            // heap info (for memory tracking...)
            size_t free_heap = xPortGetFreeHeapSize();
            size_t min_free_heap = xPortGetMinimumEverFreeHeapSize();
            printf("end - free heap: %u, min free heap: %u\n", free_heap, min_free_heap);

            return -1;
        }
        case MAIN_MESSAGE_TRIGGER_REFRESH: {
            resource_unload_all();
            return _load_startup_file();
        }
        default:
            break;
    }
    return -1;
}

static int (*_main_states[MAIN_STATE_MAX])(uint8_t * message, size_t len) = {
        [MAIN_STATE_INIT] = _state_init,
        [MAIN_STATE_INIT_ERROR] = _state_init_failure,
        // Safe Mode
        [MAIN_STATE_CONNECT] = _state_connect,
        [MAIN_STATE_SYNC_TIME] = _state_refresh_time,
        [MAIN_STATE_REFRESH_CONFIG] = _state_refresh_config,
        [MAIN_STATE_UPDATE_CHECK] = _state_ota_update_check,
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

    app_hal_init();
    fs_mount();

    // TODO What if there is not an SD card? need to use internal only
    cryptography_init();
    if (!cryptography_rsa_exists(INTERNAL_MOUNT_POINT "private.pem", SD_MOUNT_POINT "public.pem", SD_MOUNT_POINT "key_uuid.toml")) {
        cryptography_rsa_generate(INTERNAL_MOUNT_POINT "private.pem",  SD_MOUNT_POINT "public.pem", SD_MOUNT_POINT "key_uuid.toml");
    }

    toml_set_memutil(pvPortMalloc, vPortFree);
    toml_set_futil( fs_feof, toml_fs_read);

    message_buffer = xMessageBufferCreate(MAIN_MESSAGE_BUFFER_SIZE);
    uint8_t startMessage = MAIN_MESSAGE_LOAD_STARTUP;
    xMessageBufferSend(message_buffer, &startMessage, 1, portMAX_DELAY);

    wifi_task_start();

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

