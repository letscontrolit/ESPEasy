#include "../wifi/ESPEasyWiFi_STA_Event_ESP32.h"

#if FEATURE_WIFI

# ifdef ESP32
#  if FEATURE_ETHERNET
#   include <ETH.h>
#  endif // if FEATURE_ETHERNET
#  if FEATURE_PPP_MODEM
#   include <PPP.h>
#  endif

#  include "../../../src/DataStructs/RTCStruct.h"
#  include "../../../src/DataTypes/ESPEasyTimeSource.h"
#  include "../../../src/ESPEasyCore/ESPEasy_Log.h"
#  include "../../../src/Globals/RTC.h"
#  include "../../../src/Helpers/ESPEasy_time_calc.h"
#  include "../../../src/Helpers/StringConverter.h"
#  include "../../../src/Helpers/StringGenerator_WiFi.h"
#  include "../../net/ESPEasyNetwork.h"
#  include "../DataStructs/NWPluginData_static_runtime.h"
#  include "../Globals/ESPEasyWiFiEvent.h"
#  include "../Globals/NetworkState.h"
#  include "../Globals/WiFi_AP_Candidates.h"
#  include "../eth/ESPEasyEth.h"
#  include "../wifi/ESPEasyWifi.h"

namespace ESPEasy {
namespace net {
namespace wifi {

static NWPluginData_static_runtime stats_and_cache(&WiFi.STA, F("WiFi"));

static wifi_event_sta_connected_t _wifi_event_sta_connected;
static WiFiDisconnectReason _wifi_disconnect_reason = WiFiDisconnectReason::WIFI_DISCONNECT_REASON_UNSPECIFIED;

static bool _ESPEasyWiFi_STA_EventHandler_initialized{};

ESPEasyWiFi_STA_EventHandler::ESPEasyWiFi_STA_EventHandler(networkIndex_t networkIndex)
{
  stats_and_cache.clear(networkIndex);
  nw_event_id = Network.onEvent(ESPEasyWiFi_STA_EventHandler::WiFiEvent);
  memset(&_wifi_event_sta_connected, 0, sizeof(_wifi_event_sta_connected));
  _ESPEasyWiFi_STA_EventHandler_initialized = true;
}

ESPEasyWiFi_STA_EventHandler::~ESPEasyWiFi_STA_EventHandler()
{
  if (nw_event_id != 0) {
    Network.removeEvent(nw_event_id);
  }
  nw_event_id                               = 0;
  _ESPEasyWiFi_STA_EventHandler_initialized = false;
  stats_and_cache.processEvent_and_clear();
}

bool ESPEasyWiFi_STA_EventHandler::initialized()                                             {
  return _ESPEasyWiFi_STA_EventHandler_initialized;
}

NWPluginData_static_runtime * ESPEasyWiFi_STA_EventHandler::getNWPluginData_static_runtime() { return &stats_and_cache; }

WiFiDisconnectReason          ESPEasyWiFi_STA_EventHandler::getLastDisconnectReason() const  { return _wifi_disconnect_reason; }

uint8_t                       ESPEasyWiFi_STA_EventHandler::getAuthMode() const              { return _wifi_event_sta_connected.authmode; }

bool                          ESPEasyWiFi_STA_EventHandler::restore_dns_from_cache() const
{
  bool res{};

  if (WiFi.STA.isDefault()) {
    // Check to see if we may need to restore any cached DNS server
    for (size_t i = 0; i < NR_ELEMENTS(stats_and_cache._dns_cache); ++i) {
      auto tmp = WiFi.STA.dnsIP(i);

      if ((stats_and_cache._dns_cache[i] != INADDR_NONE) && (stats_and_cache._dns_cache[i] != tmp)) {
        addLog(LOG_LEVEL_INFO, strformat(
                 F("WiFi STA: Restore cached DNS server %d from %s to %s"),
                 i,
                 tmp.toString().c_str(),
                 stats_and_cache._dns_cache[i].toString().c_str()
                 ));
        WiFi.STA.dnsIP(i, stats_and_cache._dns_cache[i]);
        res = true;
      }
    }
  }
  return res;
}

const __FlashStringHelper * ESPEasyWiFi_STA_EventHandler::getWiFi_encryptionType() const
{
  return WiFi_encryptionType(_wifi_event_sta_connected.authmode);
}

// ********************************************************************************
// Functions called on events.
// Make sure not to call anything in these functions that result in delay() or yield()
// ********************************************************************************
#  include <WiFi.h>

static bool ignoreDisconnectEvent = false;

void ESPEasyWiFi_STA_EventHandler::WiFiEvent(WiFiEvent_t event_id, arduino_event_info_t info) {
  bool clear_wifi_event_sta_connected = false;

  switch (event_id)
  {
    case ARDUINO_EVENT_WIFI_OFF:
      clear_wifi_event_sta_connected = true;
      addLog(LOG_LEVEL_INFO, F("WIFI_OFF"));
      break;
    case ARDUINO_EVENT_WIFI_READY:
      //    clear_wifi_event_sta_connected = true;
      addLog(LOG_LEVEL_INFO, F("WIFI_READY"));
      break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
      addLog(LOG_LEVEL_INFO, F("SCAN_DONE"));
      break;
    case ARDUINO_EVENT_WIFI_STA_START:
      clear_wifi_event_sta_connected = true;
      stats_and_cache.mark_start();
      break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
      clear_wifi_event_sta_connected = true;
      stats_and_cache.mark_stop();
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      memcpy(&_wifi_event_sta_connected, &info.wifi_sta_connected, sizeof(_wifi_event_sta_connected));
      stats_and_cache.mark_connected();
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      clear_wifi_event_sta_connected = true;
      _wifi_disconnect_reason        = static_cast<WiFiDisconnectReason>(info.wifi_sta_disconnected.reason);
      stats_and_cache.mark_disconnected();
      break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
      _wifi_event_sta_connected.authmode = info.wifi_sta_authmode_change.new_mode;
      addLog(LOG_LEVEL_INFO, F("STA_AUTHMODE_CHANGE"));
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      stats_and_cache.mark_got_IP();
      break;
#  if FEATURE_USE_IPV6
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
      stats_and_cache.mark_got_IPv6(&info.got_ip6);
      break;
#  endif // if FEATURE_USE_IPV6
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
      stats_and_cache.mark_lost_IP();
      break;

    default:
      break;
  }

  if (clear_wifi_event_sta_connected) {
    memset(&_wifi_event_sta_connected, 0, sizeof(_wifi_event_sta_connected));
  }

  /*
     switch (event_id)
     {
      case ARDUINO_EVENT_WIFI_READY:
        // ESP32 WiFi ready
        break;
      case ARDUINO_EVENT_WIFI_STA_START:
   #  ifndef BUILD_NO_DEBUG

        // addLog(LOG_LEVEL_INFO, F("WiFi : Event STA Started"));
   #  endif // ifndef BUILD_NO_DEBUG
        break;
      case ARDUINO_EVENT_WIFI_STA_STOP:
   #  ifndef BUILD_NO_DEBUG

        // addLog(LOG_LEVEL_INFO, F("WiFi : Event STA Stopped"));
   #  endif // ifndef BUILD_NO_DEBUG
        break;
      case ARDUINO_EVENT_WIFI_OFF:
   #  ifndef BUILD_NO_DEBUG

        // addLog(LOG_LEVEL_INFO, F("WiFi : Event WIFI off"));
   #  endif // ifndef BUILD_NO_DEBUG
        break;

      case ARDUINO_EVENT_WIFI_AP_START:
   #  ifndef BUILD_NO_DEBUG

        // addLog(LOG_LEVEL_INFO, F("WiFi : Event AP Started"));
   #  endif // ifndef BUILD_NO_DEBUG
        break;
      case ARDUINO_EVENT_WIFI_AP_STOP:
   #  ifndef BUILD_NO_DEBUG

        // addLog(LOG_LEVEL_INFO, F("WiFi : Event AP Stopped"));
   #  endif // ifndef BUILD_NO_DEBUG
        break;
      case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        // ESP32 station lost IP and the IP is reset to 0
   #  if FEATURE_ETHERNET

        if (active_network_medium == NetworkMedium_t::Ethernet) {
          // DNS records are shared among WiFi and Ethernet (very bad design!)
          // So we must restore the DNS records for Ethernet in case we started with WiFi and then plugged in Ethernet.
          // As soon as WiFi is turned off, the DNS entry for Ethernet is cleared.
          EthEventData.markLostIP();
        }
   #  endif // if FEATURE_ETHERNET
        WiFiEventData.markLostIP();
   #  ifndef BUILD_NO_DEBUG

        // addLog(LOG_LEVEL_INFO,

        //         F("WiFi : Event Lost IP"));
   #  endif // ifndef BUILD_NO_DEBUG
        break;

      case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
        // Receive probe request packet in soft-AP interface
        // TODO TD-er: Must implement like onProbeRequestAPmode for ESP8266
   #  ifndef BUILD_NO_DEBUG

        // addLog(LOG_LEVEL_INFO, F("WiFi : Event AP got probed"));
   #  endif // ifndef BUILD_NO_DEBUG
        break;

      case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        WiFiEventData.setAuthMode(info.wifi_sta_authmode_change.new_mode);
        break;

      case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      {
        char ssid_copy[33]; // Ensure space for maximum len SSID (32) plus trailing 0
        memcpy(ssid_copy, info.wifi_sta_connected.ssid, info.wifi_sta_connected.ssid_len);
        ssid_copy[32] = 0;  // Potentially add 0-termination if none present earlier
        WiFiEventData.markConnected((const char *)ssid_copy, info.wifi_sta_connected.bssid, info.wifi_sta_connected.channel);
        WiFiEventData.setAuthMode(info.wifi_sta_connected.authmode);

        // addLog(LOG_LEVEL_INFO, F("WiFi : Event WIFI_STA_CONNECTED"));
        break;
      }
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        WiFiEventData.markDisconnect(static_cast<WiFiDisconnectReason>(info.wifi_sta_disconnected.reason));
        break;
      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      {
        // FIXME TD-er: Must also check event->esp_netif to see which interface got this event.
        ignoreDisconnectEvent = false;
        ip_event_got_ip_t *event = static_cast<ip_event_got_ip_t *>(&info.got_ip);
        const IPAddress    ip(event->ip_info.ip.addr);
        const IPAddress    netmask(event->ip_info.netmask.addr);
        const IPAddress    gw(event->ip_info.gw.addr);
        WiFiEventData.markGotIP(ip, netmask, gw);
        break;
      }
   #  if FEATURE_USE_IPV6
      case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
      {
        ip_event_got_ip6_t *event = static_cast<ip_event_got_ip6_t *>(&info.got_ip6);
        const IPAddress     ip(IPv6, (const uint8_t *)event->ip6_info.ip.addr, event->ip6_info.ip.zone);
        WiFiEventData.markGotIPv6(ip);
        break;
      }
      case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
        addLog(LOG_LEVEL_INFO, F("WIFI : AP got IP6"));
        break;
   #  endif // if FEATURE_USE_IPV6
      case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        addLog(LOG_LEVEL_INFO, F("WIFI : AP assigned IP to STA"));
        break;
      case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        WiFiEventData.markConnectedAPmode(info.wifi_ap_staconnected.mac);
        break;
      case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        WiFiEventData.markDisconnectedAPmode(info.wifi_ap_stadisconnected.mac);
        break;
      case ARDUINO_EVENT_WIFI_SCAN_DONE:
        break;
   #  if FEATURE_ETHERNET
      case ARDUINO_EVENT_ETH_START:
      case ARDUINO_EVENT_ETH_CONNECTED:
      case ARDUINO_EVENT_ETH_GOT_IP:
      case ARDUINO_EVENT_ETH_DISCONNECTED:
      case ARDUINO_EVENT_ETH_STOP:
      case ARDUINO_EVENT_ETH_GOT_IP6:

        // Handled in EthEvent
        break;
   #  endif // FEATURE_ETHERNET

   #  if FEATURE_PPP_MODEM

      case ARDUINO_EVENT_PPP_START:
      case ARDUINO_EVENT_PPP_CONNECTED:
      case ARDUINO_EVENT_PPP_GOT_IP:
      case ARDUINO_EVENT_PPP_LOST_IP:
      case ARDUINO_EVENT_PPP_DISCONNECTED:
      case ARDUINO_EVENT_PPP_STOP:
        // Handled in ESPEasyWiFi_STA_EventHandler
        break;

   #  endif // if FEATURE_PPP_MODEM

      default:
      {
        addLogMove(LOG_LEVEL_ERROR, concat(F("UNKNOWN WIFI/ETH EVENT: "),  event_id));
        break;
      }
     }
   */
}

} // namespace wifi
} // namespace net
} // namespace ESPEasy
# endif // ifdef ESP32
#endif // if FEATURE_WIFI
