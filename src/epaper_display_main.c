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
#include "bluetooth_task.h"
#include "filesystem_hal.h"
#include "toml.h"
#include "cryptography_hal.h"

#include "apps.h"
#include "message_box_app.h"

#include "init_hal.h"

#define NUMBER_OF_APPS (1)

#define MAIN_MESSAGE_BUFFER_SIZE (400)
#define MAIN_MESSAGE_MAX_SIZE (100)

#define STARTUP_FILENAME "startup.toml"
#define APP_CONFIG_FILENAME "config.toml"

#define AUTHOR_SUBSTITUTION_VALUE "!!MSG_BOX_AUTHOR!!"
#define MESSAGE_SUBSTITUTION_VALUE "!!MSG_BOX_MESSAGE!!"


typedef enum {
    // Checj startup configuration
    MAIN_STATE_INIT,
    MAIN_STATE_INIT_ERROR,
    // Safe mode - don't load anything
    MAIN_STATE_SAFE,
    // WiFi Connection
    MAIN_STATE_CONNECT,
    // Checking / getting auth to the server
    MAIN_STATE_AUTHENTICATION,
    //
    MAIN_STATE_REFRESH_CONFIG,
    MAIN_STATE_RUN_APP,
    MAIN_STATE_MAX,
} MAIN_STATE;

typedef enum {
    MAIN_MESSAGE_LOAD_STARTUP,
    MAIN_MESSAGE_REFRESH_READY,
    MAIN_MESSAGE_APP_INIT,
    MAIN_MESSAGE_APP,
} MAIN_STATE_MESSAGE;

typedef enum {
    MAIN_APP_TIMER_PROC,
    MAIN_APP_CUSTOM_ID_START
} MAIN_APP_MESSAGE;

static MessageBufferHandle_t message_buffer;
toml_table_t *startup_config;
toml_table_t *device_config;

static const APP_INTERFACE *_apps[NUMBER_OF_APPS] = {
    &g_message_box_interface
};
static const APP_INTERFACE *_currentApp = NULL;
static void* _currentAppContext = NULL;
static xTimerHandle _currentAppTimer = NULL;

static void _app_timer_callback(TimerHandle_t timer) {
    uint8_t message[2] = {MAIN_MESSAGE_APP, MAIN_APP_TIMER_PROC};
    xMessageBufferSend(message_buffer, &message, 2, 0);
}

static int _load_startup_file(void) {

    if (startup_config) {
        toml_free(startup_config);
        startup_config = NULL;
    }

    file_handle fh = FS_Open(STARTUP_FILENAME, "r");
    if (fh == NULL) {
        printf("No TOML file!\n");
        return MAIN_STATE_INIT_ERROR;
    }

    char toml_error_msg[100];
    startup_config = toml_parse_file(fh, toml_error_msg, 100);
    FS_Close(fh);
    if (!startup_config) {
         printf("Toml file load error: %s\n", toml_error_msg);
         return MAIN_STATE_INIT_ERROR;
    }

    toml_datum_t mode = toml_string_in(startup_config, "mode");
    if (!mode.ok) {
        printf("Mode unspecified\n");
        return MAIN_STATE_INIT_ERROR;
    }

    printf("Toml startup file loaded!\n");
    if (strcmp("standalone", mode.u.s) == 0) {

        // Standalone mode - we should expect config to already exist!
        uint8_t message = MAIN_MESSAGE_REFRESH_READY;
        xMessageBufferSend(message_buffer, &message, 1, portMAX_DELAY);

        return MAIN_STATE_REFRESH_CONFIG;
    } else if (strcmp("online", mode.u.s) == 0) {
        // Online mode - we should connect to WiFi
        toml_array_t* wifi_credentials = toml_array_in(startup_config, "wifi");
        int num_wifi_aps = toml_array_nelem(wifi_credentials);
        for (int i=0; i<num_wifi_aps; i++) {
            toml_table_t *current_creds = toml_table_at(wifi_credentials, i);

            toml_datum_t ssid = toml_string_in(current_creds, "ssid");
            toml_datum_t password = toml_string_in(current_creds, "password");

            if (!ssid.ok || !password.ok) {
                return MAIN_STATE_INIT_ERROR;
            }

            printf("loaded wifi credentials: %s, %s\n", ssid.u.s, password.u.s);
        }

        // TODO Send message to connect with credentials to Wifi Task
        return MAIN_STATE_CONNECT;
    } else {
        printf("Unknown run mode\n");
        return MAIN_STATE_INIT_ERROR;
    }

}

static int _load_config_file(void) {

    if (device_config) {
        toml_free(device_config);
        device_config = NULL;
    }

    file_handle fh = FS_Open(APP_CONFIG_FILENAME, "r");
    if (fh == NULL) {
        return MAIN_STATE_INIT_ERROR;
    }

    char toml_error_msg[100];
    device_config = toml_parse_file(fh, toml_error_msg, 100);
    FS_Close(fh);
    if (!device_config) {
        printf("Toml file load error: %s\n", toml_error_msg);
        return MAIN_STATE_INIT_ERROR;
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

static int _state_refresh_config(uint8_t *message, size_t len) {
    switch(message[0]) {
        case MAIN_MESSAGE_REFRESH_READY:
            return _load_config_file();
        default:
            break;
    }
    return -1;
}

static int _state_run_app(uint8_t *message, size_t len) {
    switch(message[0]) {
        case MAIN_MESSAGE_APP_INIT: {
            _currentApp = NULL;
            toml_table_t *app_info = toml_table_in(device_config, "application");
            toml_datum_t app_name = toml_string_in(app_info, "name");

            for (int i=0; i<NUMBER_OF_APPS; i++) {
                if (strcmp(app_name.u.s, _apps[i]->name) == 0) {
                    _currentApp = _apps[i];
                    break;
                }
            }
            if (_currentApp) {
                _currentAppContext = _currentApp->app_init(startup_config, device_config);
                if (!_currentAppTimer) {
                    _currentAppTimer = xTimerCreate("App", _currentApp->refresh_rate_ms*1000/configTICK_RATE_HZ, pdTRUE,
                                                    NULL, _app_timer_callback);
                } else {
                    xTimerChangePeriod(_currentAppTimer, _currentApp->refresh_rate_ms*1000/configTICK_RATE_HZ, portMAX_DELAY);
                }
                xTimerStart(_currentAppTimer, portMAX_DELAY);
            }
        }
            break;
        case MAIN_MESSAGE_APP: {
            if (_currentApp) {
                _currentApp->app_process(_currentAppContext, message+1, len-1);
            }
        }
        default:
            break;
    }
    return -1;
}

static int (*_main_states[MAIN_STATE_MAX])(uint8_t * message, size_t len) = {
        [MAIN_STATE_INIT] = _state_init,
        [MAIN_STATE_REFRESH_CONFIG] = _state_refresh_config,
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

    const char* shatest = "asdfasdfasdfasdfasdf;jlkasldkjglaj;sdlfjasldkjg";
    uint8_t digest[32] = {0};
    cryptography_digest_sha((const uint8_t*) shatest, strlen(shatest), 256, digest);

    printf("Tested SHA\n");

    uint8_t *output = NULL;
    size_t output_len = 0;
    cryptography_sign_rsa("private.pem", digest, 32, &output, &output_len);

    printf("output: %p, %zu\n", output, output_len);

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
            printf("No message received\n");
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
