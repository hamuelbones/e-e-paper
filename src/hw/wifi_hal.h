
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void WIFI_Init(void);

bool WIFI_Connect(const char* ssid, const char* password);
void WIFI_Disconnect(void);

bool WIFI_HttpGet(const char* host,
                  const char* subdirectory,
                  const char** headers,
                  size_t header_count,

                  const char* headers_filename,
                  const char* response_filename,
                  int *status);

bool WIFI_Connected(void);

uint32_t WIFI_GetNetworkTime(const char* host);
