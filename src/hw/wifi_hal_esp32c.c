
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "wifi_hal.h"

#include "esp_event_base.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_http_client.h"

static EventGroupHandle_t _message_buffer;

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 10) {
            esp_wifi_connect();
            s_retry_num++;
        } else {
            char message = WIFI_HAL_CONNECTION_FAILED;
            xMessageBufferSend(_message_buffer, &message, 1, portMAX_DELAY);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        s_retry_num = 0;

        char message = WIFI_HAL_CONNECTED;
        xMessageBufferSend(_message_buffer, &message, 1, portMAX_DELAY);
    }
}



void WIFI_Init(MessageBufferHandle_t message_buffer) {

    _message_buffer = message_buffer;

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
}

void WIFI_Connect(const char* ssid, const char* password) {

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
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

}