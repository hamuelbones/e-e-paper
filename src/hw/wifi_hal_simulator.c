//
// Created by Samuel Jones on 1/21/22.
//

#include "wifi_hal.h"
#include "filesystem_hal.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>


static SSL_CTX *ssl;

void WIFI_Init(void) {

    SSL_library_init();
    SSL_load_error_strings();
    ssl = SSL_CTX_new(TLS_client_method());
}

bool WIFI_Connect(const char* ssid, const char* password) {
    return true;
}

bool WIFI_Connected(void) {
    return true;
}

void WIFI_Disconnect() {

}

void error( char* msg )
{
    perror( msg ); // Print the error message to stderr.

    exit( 0 ); // Quit the process.
}
#define NTP_TIMESTAMP_DELTA 2208988800ull

uint32_t WIFI_GetNetworkTime(const char* host) {

    // From https://github.com/lettier/ntpclient/blob/master/source/c/main.c
    int sockfd, n; // Socket file descriptor and the n return result from writing/reading from the socket.

    // Structure that defines the 48 byte NTP packet protocol.

    typedef struct
    {

        uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
        // li.   Two bits.   Leap indicator.
        // vn.   Three bits. Version number of the protocol.
        // mode. Three bits. Client will pick mode 3 for client.

        uint8_t stratum;         // Eight bits. Stratum level of the local clock.
        uint8_t poll;            // Eight bits. Maximum interval between successive messages.
        uint8_t precision;       // Eight bits. Precision of the local clock.

        uint32_t rootDelay;      // 32 bits. Total round trip delay time.
        uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
        uint32_t refId;          // 32 bits. Reference clock identifier.

        uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
        uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

        uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
        uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

        uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
        uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

        uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
        uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

    } ntp_packet;              // Total: 384 bits or 48 bytes.

    // Create and zero out the packet. All 48 bytes worth.

    ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    memset( &packet, 0, sizeof( ntp_packet ) );

    // Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.

    *( ( char * ) &packet + 0 ) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.

    // Create a UDP socket, convert the host-name to an IP address, set the port number,
    // connect to the server, send the packet, and then read in the return packet.

    struct sockaddr_in serv_addr; // Server address data structure.
    struct hostent *server;      // Server data structure.

    sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ); // Create a UDP socket.

    if ( sockfd < 0 )
        error( "ERROR opening socket" );

    server = gethostbyname( host ); // Convert URL to IP.

    if ( server == NULL )
        error( "ERROR, no such host" );

    // Zero out the server address structure.

    bzero( ( char* ) &serv_addr, sizeof( serv_addr ) );

    serv_addr.sin_family = AF_INET;

    // Copy the server's IP address to the server address structure.

    bcopy( ( char* )server->h_addr, ( char* ) &serv_addr.sin_addr.s_addr, server->h_length );

    // Convert the port number integer to network big-endian style and save it to the server address structure.

    serv_addr.sin_port = htons( 123 );

    // Call up the server using its IP address and port number.

    if ( connect( sockfd, ( struct sockaddr * ) &serv_addr, sizeof( serv_addr) ) < 0 )
        error( "ERROR connecting" );

    // Send it the NTP packet it wants. If n == -1, it failed.

    n = write( sockfd, ( char* ) &packet, sizeof( ntp_packet ) );

    if ( n < 0 )
        error( "ERROR writing to socket" );


    while (1) {

        n = read( sockfd, ( char* ) &packet, sizeof( ntp_packet ) );
        if ( n < 0 ) {
            if (errno != EINTR) {
                error( "ERROR reading from socket" );
            }
            printf("EINTR\n");
            usleep(1000);
        } else {
            break;
        }
    }

    // These two fields contain the time-stamp seconds as the packet left the NTP server.
    // The number of seconds correspond to the seconds passed since 1900.
    // ntohl() converts the bit/byte order from the network's to host's "endianness".

    packet.txTm_s = ntohl( packet.txTm_s ); // Time-stamp seconds.
    packet.txTm_f = ntohl( packet.txTm_f ); // Time-stamp fraction of a second.

    // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server.
    // Subtract 70 years worth of seconds from the seconds since 1900.
    // This leaves the seconds since the UNIX epoch of 1970.
    // (1900)------------------(1970)**************************************(Time Packet Left the Server)

    time_t txTm = ( time_t ) ( packet.txTm_s - NTP_TIMESTAMP_DELTA );

    return (uint32_t) txTm;
}

typedef enum {
    PARSE_RESPONSE_CODE,
    PARSE_HEADERS,
    PARSE_RESPONSE,
} HTTP_PARSE_STATE;


bool WIFI_HttpGet(const char* host,
                  const char* subdirectory,
                  const char** headers,
                  size_t header_count,

                  const char* headers_filename,
                  const char* response_filename,
                  int *status) {

    BIO * bio = BIO_new_ssl_connect(ssl);
    SSL * thisSSL;
    BIO_get_ssl(bio, &thisSSL);
    SSL_set_mode(thisSSL, SSL_MODE_AUTO_RETRY);

    BIO_set_conn_hostname(bio, host);

    int result = 0;
    do {
        result = BIO_do_connect(bio);
    } while (result <= 0 && BIO_should_retry(bio));

    if (result <= 0) {
        printf("Failed connection %d\n", result);
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        return false;
    }

    if(BIO_do_handshake(bio) <= 0) {
        fprintf(stderr, "Error establishing SSL connection\n");
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        return false;
    }

    char line[200] = {0};
    int len = 0;
    len = snprintf(line, 200, "GET %s HTTP/1.1\r\n", subdirectory);
    BIO_write(bio, line, len);
    len = snprintf(line, 200, "Host: %s\r\n", host);
    BIO_write(bio, line, len);
    BIO_puts(bio, "Connection: close\r\n");
    for (size_t i = 0; i<header_count; i++) {
        BIO_puts(bio, headers[i]);
        BIO_puts(bio, "\r\n");
    }
    BIO_puts(bio, "\r\n");
    BIO_flush(bio);

    int max_stall_count = 1000;
    int stall_count = 0;
    int us_per_count = 1000;

    HTTP_PARSE_STATE state = PARSE_RESPONSE_CODE;

    char response_data[128];

    size_t line_len = 0;
    bool http_version_complete = false;
    bool http_status_complete = false;

    *status = 0;

    file_handle header_file = NULL;
    file_handle response_file = NULL;

    while (1) {
        int size = BIO_read(bio, response_data, 128);

        if (size > 0) {
            stall_count = 0;
        } else {
            if (stall_count++ >= max_stall_count) {
                break;
            }
            usleep(us_per_count);
            continue;
        }

        // Sort of hacky, perhaps parse better in the future or use a library
        for (int i=0; i<size; i++) {
            char c = response_data[i];
            line_len++;

            switch (state) {
                case PARSE_RESPONSE_CODE:
                    // We will be here for the first line.
                    if (c == ' ') {
                        // First space: next value is HTTP status code
                        if (!http_version_complete) {
                            http_version_complete = true;
                        } else {
                            http_status_complete = true;
                        }
                    } else {
                        if (http_version_complete && !http_status_complete) {
                            *status = (*status * 10);
                            *status += c - '0';
                        }
                    }

                    if (c == '\n') {
                        state = PARSE_HEADERS;
                        if (headers_filename) {
                            FS_Remove(headers_filename);
                            header_file = FS_Open(headers_filename, "w");
                        }
                    }
                    break;
                case PARSE_HEADERS:
                    if (header_file) {
                        FS_Write(header_file, &c, 1);
                    }
                    if (c == '\n' && line_len <= 2) {
                        state = PARSE_RESPONSE;
                        if (header_file) {
                            FS_Close(header_file);
                        }
                        if (response_filename) {
                            FS_Remove(response_filename);
                            response_file = FS_Open(response_filename, "w");
                        }
                    }

                    break;
                case PARSE_RESPONSE:
                    if (response_file) {
                        FS_Write(response_file, &c, 1);
                    }
                    break;
            }

            if (c == '\n') {
                line_len = 0;
            }
        }
    }
    if (response_file) {
        FS_Close(response_file);
    }

    BIO_free_all(bio);
    return true;
}