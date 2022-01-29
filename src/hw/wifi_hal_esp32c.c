
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "wifi_hal.h"

#include "esp_event_base.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_http_client.h"

static EventGroupHandle_t _event_group;
static TimerHandle_t _timeout_timer;

typedef enum {
    ESP_WIFI_CONNECTED = (1<<0),
    ESP_WIFI_DISCONNECTED = (1<<1),
    ESP_WIFI_STARTED = (1<<2),
    ESP_WIFI_STOPPED = (1<<3),
    ESP_WIFI_TIMEOUT = (1<<4),
} ESP_WIFI_BITS;

static void start_timeout_timer(uint32_t ms) {
    xTimerChangePeriod(_timeout_timer, ms/portTICK_RATE_MS, portMAX_DELAY);
}

static void stop_timeout_timer(void) {
    xTimerStop(_timeout_timer, portMAX_DELAY);
}

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        xEventGroupSetBits(_event_group, ESP_WIFI_STARTED);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_STOP) {
        xEventGroupSetBits(_event_group, ESP_WIFI_STOPPED);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupSetBits(_event_group, ESP_WIFI_DISCONNECTED);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(_event_group, ESP_WIFI_CONNECTED);
    }
}

static void timeout_handler(TimerHandle_t xTimer) {
    xEventGroupSetBits(_event_group, ESP_WIFI_TIMEOUT);
}



void WIFI_Init(void) {

    xTimerCreate("wifi_tmo", 100, false, NULL, timeout_handler);
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

    // Wait for connection
    start_timeout_timer(10000);
    xEventGroupClearBits(_event_group, ESP_WIFI_CONNECTED | ESP_WIFI_TIMEOUT | ESP_WIFI_DISCONNECTED);
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

    esp_err_t err;

    esp_http_client_config_t config = {

    };
    //err = esp_http_client_init()

    return false;
}

uint32_t WIFI_GetNetworkTime(const char* host) {

    return 0;
}