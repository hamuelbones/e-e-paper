
#include "FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/message_buffer.h"

typedef enum {
    WIFI_HAL_CONNECTED,
    WIFI_HAL_CONNECTION_FAILED,
    WIFI_HAL_DISCONNECTED,

    WIFI_HAL_HTTP_RESPONSE,
    WIFI_HAL_HTTP_ERROR,
} WIFI_HAL_MESSAGE;

#define INVALID_HTTP_REQ_ID (-1)
typedef int WIFI_HTTP_REQ_ID;

void WIFI_Init(MessageBufferHandle_t message_buffer);

void WIFI_Connect(const char* ssid, const char* password);
WIFI_HTTP_REQ_ID WIFI_HttpGet(const char* host,
                              const char* subdirectory,
                              const char** headers,
                              size_t header_count);

WIFI_HTTP_REQ_ID WIFI_StartHTTPRequest();