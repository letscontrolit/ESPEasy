#include "../DataTypes/TLS_types.h"

const __FlashStringHelper* toString(TLS_types tls_type)
{
  switch (tls_type) {
    case TLS_types::NoTLS:        break;
    case TLS_types::TLS_PSK:      return F("TLS PreSharedKey");
    case TLS_types::TLS_CA_CERT:  return F("TLS CA Cert");
    case TLS_types::TLS_insecure: return F("TLS No Checks (insecure)");
    case TLS_types::TLS_FINGERPRINT: return F("TLS Certficate Fingerprint");
  }
  return F("No TLS");
}
