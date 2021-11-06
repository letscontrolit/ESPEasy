/* Provide SSL/TLS functions to ESP32 with Arduino IDE
 * by Evandro Copercini - 2017 - Apache 2.0 License
 */

#ifndef ESPEASY_ARD_SSL_H
#define ESPEASY_ARD_SSL_H
#include <mbedtls/platform.h>
#include <mbedtls/net.h>
#include <mbedtls/debug.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>

typedef struct ESPEasy_sslclient_context {

    ESPEasy_sslclient_context();

    ~ESPEasy_sslclient_context();

    void free_ca_cert();
    void free_client_cert();
    void free_client_key();

    int socket = -1;
    mbedtls_ssl_context ssl_ctx;
    mbedtls_ssl_config ssl_conf;

    mbedtls_ctr_drbg_context drbg_ctx;
    mbedtls_entropy_context entropy_ctx;

    mbedtls_x509_crt ca_cert;
    mbedtls_x509_crt client_cert;
    mbedtls_pk_context client_key;

    unsigned long handshake_timeout = 120000;
} ESPEasy_sslclient_context;


void ssl_init(ESPEasy_sslclient_context *ssl_client);
int start_ssl_client(ESPEasy_sslclient_context *ssl_client, const char *host, uint32_t port, int timeout, const char *rootCABuff, const char *cli_cert, const char *cli_key, const char *pskIdent, const char *psKey, bool insecure, const char **alpn_protos);
void stop_ssl_socket(ESPEasy_sslclient_context *ssl_client, const char *rootCABuff, const char *cli_cert, const char *cli_key);
int data_to_read(ESPEasy_sslclient_context *ssl_client);
int send_ssl_data(ESPEasy_sslclient_context *ssl_client, const uint8_t *data, uint16_t len);
int get_ssl_receive(ESPEasy_sslclient_context *ssl_client, uint8_t *data, int length);
bool verify_ssl_fingerprint(ESPEasy_sslclient_context *ssl_client, const char* fp, const char* domain_name);
bool verify_ssl_dn(ESPEasy_sslclient_context *ssl_client, const char* domain_name);
bool get_peer_fingerprint(ESPEasy_sslclient_context *ssl_client, uint8_t sha256[32]);

#endif