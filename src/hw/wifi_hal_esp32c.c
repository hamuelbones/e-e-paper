
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "wifi_hal.h"

#include "esp_event_base.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_http_client.h"
#include "filesystem_hal.h"

#include "sntp.h"

static EventGroupHandle_t _event_group;
static TimerHandle_t _timeout_timer;

typedef enum {
    ESP_WIFI_CONNECTED = (1<<0),
    ESP_WIFI_DISCONNECTED = (1<<1),
    ESP_WIFI_STARTED = (1<<2),
    ESP_WIFI_STOPPED = (1<<3),
    ESP_WIFI_TIMEOUT = (1<<4),
    ESP_WIFI_TIME_SYNC = (1<<5),
} ESP_WIFI_BITS;

static void start_timeout_timer(uint32_t ms) {
    xTimerChangePeriod(_timeout_timer, ms/portTICK_RATE_MS, portMAX_DELAY);
    xTimerStart(_timeout_timer, portMAX_DELAY);
}

static void stop_timeout_timer(void) {
    xTimerStop(_timeout_timer, portMAX_DELAY);
}

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    printf("base: %s, id: %d\n", event_base, event_id);
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        xEventGroupSetBits(_event_group, ESP_WIFI_STARTED);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_STOP) {
        xEventGroupSetBits(_event_group, ESP_WIFI_STOPPED);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        //xEventGroupSetBits(_event_group, ESP_WIFI_DISCONNECTED);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(_event_group, ESP_WIFI_CONNECTED);
    }
}

static void timeout_handler(TimerHandle_t xTimer) {
    xEventGroupSetBits(_event_group, ESP_WIFI_TIMEOUT);
}



void WIFI_Init(void) {

    _timeout_timer = xTimerCreate("wifi_tmo", 100, false, NULL, timeout_handler);
    _event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    xEventGroupWaitBits(_event_group, ESP_WIFI_STARTED, true, false, portMAX_DELAY);

    sntp_init();
}

bool WIFI_Connect(const char* ssid, const char* password) {

    wifi_config_t wifi_config = {
            .sta = {
                    /* Setting a password implies station will connect to all security modes including WEP/WPA.
                     * However these modes are deprecated and not advisable to be used. Incase your Access point
                     * doesn't support WPA2, these mode can be enabled by commenting below line */
                    .threshold.authmode =  password != NULL ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN,

                    .pmf_cfg = {
                            .capable = true,
                            .required = false
                    },
            },
    };
    strncpy((char*)wifi_config.sta.ssid, ssid, 31);
    strncpy((char*)wifi_config.sta.password, password, 61);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    esp_wifi_connect();

    // Wait for connection
    xEventGroupClearBits(_event_group, ESP_WIFI_CONNECTED | ESP_WIFI_TIMEOUT | ESP_WIFI_DISCONNECTED);
    start_timeout_timer(10000);
    EventBits_t bits = xEventGroupWaitBits(_event_group, ESP_WIFI_CONNECTED | ESP_WIFI_TIMEOUT,
                        true, false, portMAX_DELAY);
    stop_timeout_timer();

    if (!(bits & ESP_WIFI_CONNECTED)) {
        printf("WiFi Connect timeout!");
        strcpy((char*)wifi_config.sta.ssid, "dummy");
        strcpy((char*)wifi_config.sta.password, "dummy");
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
        return false;
    }

    sntp_init();

    return true;
}

void WIFI_Disconnect(void) {
    wifi_ap_record_t station_info;
    esp_err_t err = esp_wifi_sta_get_ap_info(&station_info);
    if (err != ESP_ERR_WIFI_NOT_CONNECT) {
        esp_wifi_disconnect();
        xEventGroupWaitBits(_event_group, ESP_WIFI_DISCONNECTED,
                            true, false, portMAX_DELAY);
    } else {
        xEventGroupClearBits(_event_group, ESP_WIFI_DISCONNECTED);
    }
}

bool WIFI_HttpGet(const char* host,
                  const char* subdirectory,
                  const char** headers,
                  size_t header_count,

                  const char* headers_filename,
                  const char* response_filename,
                  int *status) {

    //TODO Implement saving headers - not done for now

    esp_http_client_config_t config = {0};
    config.method = HTTP_METHOD_GET;
    config.host = host;
    config.path = subdirectory;

    esp_http_client_handle_t handle = esp_http_client_init(&config);


    for (int i=0; i<header_count; i++) {
        size_t header_len = strlen(headers[i]);
        char* header_copy = pvPortMalloc(header_len+1);
        strcpy(header_copy, headers[i]);
        char* kv_break = strrchr(header_copy, ':');
        *kv_break = 0;
        printf("Setting header, key (%s) value (%s)\n", header_copy, kv_break+2);
        esp_http_client_set_header(handle, header_copy, kv_break+2);

        vPortFree(header_copy);
    }

    esp_http_client_open(handle, 0);
    int content_length = esp_http_client_fetch_headers(handle);
    if (content_length < 0) {
        printf("HTTP client fetch headers failed\n");
        esp_http_client_close(handle);
        esp_http_client_cleanup(handle);
        return false;
    }
    *status = esp_http_client_get_status_code(handle);

    file_handle f = FS_Open(response_filename, "wb");
    if (!f) {
        printf("Couldn't open response file!\n");
        esp_http_client_close(handle);
        esp_http_client_cleanup(handle);
        return false;
    }

    while (1) {
        char buffer[128];
        if (esp_http_client_is_complete_data_received(handle)) {
            break;
        }
        int data_read = esp_http_client_read_response(handle, buffer, 128);
        if (data_read >= 0) {
            FS_Write(f, buffer, data_read);
        }
    }
    FS_Close(f);
    esp_http_client_close(handle);
    esp_http_client_cleanup(handle);

    return true;

}

struct timeval _time;
void _time_sync_notification(struct timeval *tv) {
    memcpy(&_time, tv, sizeof(struct timeval));
    xEventGroupSetBits(_event_group,ESP_WIFI_TIME_SYNC);
}


char *cur_ntp_server;
uint32_t WIFI_GetNetworkTime(const char* host) {

    if (cur_ntp_server) {
        vPortFree(cur_ntp_server);
    }

    size_t len = strlen(host);
    cur_ntp_server = pvPortMalloc(len+1);
    strcpy(cur_ntp_server, host);

    sntp_stop();
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, cur_ntp_server);
    sntp_set_time_sync_notification_cb(_time_sync_notification);
    sntp_init();

    printf("Waiting for time sync event\t");

    xEventGroupClearBits(_event_group, ESP_WIFI_TIME_SYNC);
    EventBits_t bits = xEventGroupWaitBits(_event_group, ESP_WIFI_TIME_SYNC,
                                           true, false, portMAX_DELAY);

    printf("Unix time: %u\t", _time.tv_sec);

    sntp_stop();
    return _time.tv_sec;
}