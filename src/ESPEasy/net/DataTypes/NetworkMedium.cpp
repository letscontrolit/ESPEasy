
#include "../DataTypes/NetworkMedium.h"

namespace ESPEasy {
namespace net {

bool isValid(ESPEasy::net::NetworkMedium_t medium) {
  switch (medium)
  {
    case ESPEasy::net::NetworkMedium_t::WIFI:
    case ESPEasy::net::NetworkMedium_t::Ethernet:
#ifdef USES_ESPEASY_NOW
    case ESPEasy::net::NetworkMedium_t::ESPEasyNOW_only:
#endif
      return true;

    case ESPEasy::net::NetworkMedium_t::NotSet:
      return false;

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return false;
}

const __FlashStringHelper* toString(ESPEasy::net::NetworkMedium_t medium) {
  switch (medium)
  {
    case ESPEasy::net::NetworkMedium_t::WIFI:     return F("WiFi");
    case ESPEasy::net::NetworkMedium_t::Ethernet: return F("Ethernet");
#ifdef USES_ESPEASY_NOW
    case ESPEasy::net::NetworkMedium_t::ESPEasyNOW_only:  return F(ESPEASY_NOW_NAME " only");
#endif
    case ESPEasy::net::NetworkMedium_t::NotSet:   return F("Not Set");

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return F("Unknown");
}

} // namespace net
} // namespace ESPEasy
