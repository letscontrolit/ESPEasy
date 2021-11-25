#ifndef DATATYPES_TLS_TYPES_H
#define DATATYPES_TLS_TYPES_H


#include <stdint.h>
#include <Arduino.h>

// Value is stored, so do not change assigned integer values.
enum class TLS_types {
  NoTLS           = 0,  // Do not use encryption
  TLS_PSK         = 1,  // Pre-Shared-Key
  TLS_CA_CERT     = 2,  // Validate server certificate against known CA
//TLS_CA_CLI_CERT = 3,  // TLS_CA_CERT + supply client certificate for authentication
  TLS_FINGERPRINT = 4,  // Use certificate fingerprint
  TLS_insecure    = 0xF // Set as last option, do not check supplied certificate. Ideal for man-in-the-middle attack.
};

const __FlashStringHelper* toString(TLS_types tls_type);


#endif // ifndef DATATYPES_TLS_TYPES_H
