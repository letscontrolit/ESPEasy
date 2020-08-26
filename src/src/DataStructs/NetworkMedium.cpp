#include "NetworkMedium.h"

bool isValid(NetworkMedium_t medium) {
  switch (medium) {
    case NetworkMedium_t::WIFI:
    case NetworkMedium_t::Ethernet:
      return true;

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return false;
}

String toString(NetworkMedium_t medium) {
  switch (medium) {
    case NetworkMedium_t::WIFI:     return F("WiFi");
    case NetworkMedium_t::Ethernet: return F("Ethernet");

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return F("Unknown");
}
