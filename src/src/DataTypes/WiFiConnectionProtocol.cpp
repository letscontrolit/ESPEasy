#include "../DataTypes/WiFiConnectionProtocol.h"

String toString(WiFiConnectionProtocol proto) {
  switch (proto) {
    case WiFiConnectionProtocol::WiFi_Protocol_11b:
      return F("802.11b");
    case WiFiConnectionProtocol::WiFi_Protocol_11g:
      return F("802.11g");
    case WiFiConnectionProtocol::WiFi_Protocol_11n:
      return F("802.11n");
    case WiFiConnectionProtocol::Unknown:
      break;;
  }
  return F("-");
}