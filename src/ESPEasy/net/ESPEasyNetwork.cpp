#include "../net/ESPEasyNetwork.h"

#include "../net/wifi/ESPEasyWifi.h"
#include "../net/Globals/NetworkState.h"

#include "../../src/Globals/ESPEasy_time.h"
#include "../../src/Globals/Settings.h"

#include "../../src/Helpers/Networking.h"
#include "../../src/Helpers/StringConverter.h"
#include "../../src/Helpers/MDNS_Helper.h"

namespace ESPEasy {
namespace net {

// ********************************************************************************
// Determine Wifi AP name to set. (also used for mDNS)
// ********************************************************************************
String NetworkGetHostNameFromSettings(bool force_add_unitnr)
{
  if (force_add_unitnr) { return Settings.getHostname(true); }
  return Settings.getHostname();
}

String NetworkCreateRFCCompliantHostname(bool force_add_unitnr) {
  // Create hostname with - instead of spaces
  return makeRFCCompliantName(NetworkGetHostNameFromSettings(force_add_unitnr));
}

String makeRFCCompliantName(const String& name, const char replaceChar, const char allowedChar, const size_t maxlength) {
  String hostname(name);

  // See RFC952. (when using the default arguments: '-', '-', 24)
  // Allowed chars:
  // * letters (a-z, A-Z)
  // * numerals (0-9)
  // * Hyphen (-)
  // * Max length 24
  replaceUnicodeByChar(hostname, replaceChar);

  for (size_t i = 0; i < hostname.length(); ++i) {
    const char c = hostname[i];

    if (!isAlphaNumeric(c) && (c != allowedChar)) {
      hostname[i] = replaceChar;
    }
  }

  // May not start or end with a hyphen
  const String dash(replaceChar);

  while (hostname.startsWith(dash)) {
    hostname = hostname.substring(1);
  }

  while (hostname.endsWith(dash)) {
    hostname = hostname.substring(0, hostname.length() - 1);
  }

  // May not contain only numerals
  bool onlyNumerals = true;

  for (size_t i = 0; onlyNumerals && i < hostname.length(); ++i) {
    const char c = hostname[i];

    if (!isdigit(c)) {
      onlyNumerals = false;
    }
  }

  if (onlyNumerals) {
    hostname = strformat(F("ESPEasy%c%s"), replaceChar, hostname.c_str());
  }

  if ((maxlength > 0) && (hostname.length() > maxlength)) {
    hostname = hostname.substring(0, maxlength);
  }

  return hostname;
}

void CheckRunningServices() {
  // First try to get the time, since that may be used in logs
  if (Settings.UseNTP() && (node_time.getTimeSource() > timeSource_t::NTP_time_source)) {
    node_time.lastNTPSyncTime_ms = 0;
    node_time.initTime();
  }
#if FEATURE_ESPEASY_P2P
    updateUDPport(true);
#endif

#if FEATURE_WIFI
# if FEATURE_SET_WIFI_TX_PWR

  if (active_network_medium == NetworkMedium_t::WIFI)
  {
    ESPEasy::net::wifi::SetWiFiTXpower();
  }
# endif // if FEATURE_SET_WIFI_TX_PWR
#endif // if FEATURE_WIFI

#if FEATURE_MDNS
  set_mDNS();
  #endif
}

uint64_t NetworkConnectDuration_ms()
{
  auto data = getDefaultRoute_NWPluginData_static_runtime();

  if (data == nullptr) {
    return 0ull;
  }
  return data->_connectedStats.getLastOnDuration_ms();
}

uint32_t NetworkConnectCount()
{
  auto data = getDefaultRoute_NWPluginData_static_runtime();

  if (data == nullptr) {
    return 0ul;
  }
  return data->_connectedStats.getCycleCount();
}


} // namespace net
} // namespace ESPEasy
