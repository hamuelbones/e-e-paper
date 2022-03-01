
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void wifi_init(void);

bool wifi_connect(const char* ssid, const char* password);
void wifi_disconnect(void);

bool wifi_http_get(const char* host,
                  const char* subdirectory,
                  const char** headers,
                  size_t header_count,
                  bool use_ssl,

                  const char* headers_filename,
                  const char* response_filename,
                  int *status);

bool wifi_connected(void);

uint32_t wifi_get_ntp(const char* host);
