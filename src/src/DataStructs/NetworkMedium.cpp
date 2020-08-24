#include "NetworkMedium.h"

bool isValid(NetworkMedium_t medium) {
  switch (medium) {
    case NetworkMedium_t::WIFI:
    case NetworkMedium_t::Ethernet:
      return true;
    default:
      return false;
  }
}

String toString(NetworkMedium_t medium) {
  switch (medium) {
    case NetworkMedium_t::WIFI:     return F("WiFi");
    case NetworkMedium_t::Ethernet: return F("Ethernet");
    default:
      return F("Unknown");
  }
}


