#include "WifiEspNowBroadcast.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <user_interface.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <esp_wifi.h>
#else
#error "This library supports ESP8266 and ESP32 only."
#endif

// #define WIFIESPNOW_DEBUG
#ifdef WIFIESPNOW_DEBUG
#define LOG(...)                                                                                   \
  do {                                                                                             \
    Serial.printf("[WifiEspNowBroadcast] " __VA_ARGS__);                                           \
    Serial.println();                                                                              \
  } while (false)
#else
#define LOG(...)                                                                                   \
  do {                                                                                             \
  } while (false)
#endif

WifiEspNowBroadcastClass WifiEspNowBroadcast;

bool
WifiEspNowBroadcastClass::begin(const char* ssid, int channel, int scanFreq)
{
  m_ssid = ssid;
  m_nextScan = 0;
  m_scanFreq = scanFreq;

  // AP mode for announcing our presence, STA mode for scanning
  WiFi.mode(WIFI_AP_STA);
  // disconnect from any previously saved SSID, so that the specified channel can take effect
  WiFi.disconnect();
  // establish AP at the specified channel to announce our presence
  WiFi.softAP(ssid, nullptr, channel);

  return WifiEspNow.begin();
}

void
WifiEspNowBroadcastClass::end()
{
  WifiEspNow.end();
  WiFi.softAPdisconnect();
  m_ssid = "";
}

void
WifiEspNowBroadcastClass::loop()
{
  if (millis() >= m_nextScan && !m_isScanning && WiFi.scanComplete() != WIFI_SCAN_RUNNING) {
    this->scan();
  }
#ifdef ARDUINO_ARCH_ESP32
  if (m_isScanning && WiFi.scanComplete() >= 0) {
    this->processScan();
  }
#endif
}

bool
WifiEspNowBroadcastClass::setKey(const uint8_t primary[WIFIESPNOW_KEYLEN],
                                 const uint8_t peer[WIFIESPNOW_KEYLEN])
{
  if (peer == nullptr) {
    m_hasPeerKey = false;
    return true;
  }
  m_hasPeerKey = true;
  std::copy_n(peer, WIFIESPNOW_KEYLEN, m_peerKey);
  return WifiEspNow.setPrimaryKey(primary);
}

void
WifiEspNowBroadcastClass::scan()
{
  LOG("scan()");
  m_isScanning = true;
#if defined(ARDUINO_ARCH_ESP8266)
  scan_config sc{};
#elif defined(ARDUINO_ARCH_ESP32)
  wifi_scan_config_t sc{};
#endif
  sc.ssid = reinterpret_cast<uint8_t*>(const_cast<char*>(m_ssid.c_str()));
#if defined(ARDUINO_ARCH_ESP8266)
  wifi_station_scan(&sc, reinterpret_cast<scan_done_cb_t>(WifiEspNowBroadcastClass::processScan));
#elif defined(ARDUINO_ARCH_ESP32)
  esp_wifi_scan_start(&sc, false);
#endif
}

#if defined(ARDUINO_ARCH_ESP8266)
void
WifiEspNowBroadcastClass::processScan(void* result, int status)
{
  WifiEspNowBroadcast.processScan2(result, status);
}

void
WifiEspNowBroadcastClass::processScan2(void* result, int status)

#define FOREACH_AP(f)                                                                              \
  do {                                                                                             \
    for (bss_info* it = reinterpret_cast<bss_info*>(result); it; it = STAILQ_NEXT(it, next)) {     \
      (f)(it->bssid, it->channel);                                                                 \
    }                                                                                              \
  } while (false)

#define DELETE_APS                                                                                 \
  do {                                                                                             \
  } while (false)

#elif defined(ARDUINO_ARCH_ESP32)
void
WifiEspNowBroadcastClass::processScan()

// ESP32 WiFiScanClass::_scanDone is always invoked after a scan complete event, so we can use
// Arduino's copy of AP records, but we must check SSID, and should not always delete AP records.

#define FOREACH_AP(f)                                                                              \
  do {                                                                                             \
    int nNetworks = WiFi.scanComplete();                                                           \
    for (uint8_t i = 0; static_cast<int>(i) < nNetworks; ++i) {                                    \
      if (WiFi.SSID(i) != m_ssid) {                                                                \
        continue;                                                                                  \
      }                                                                                            \
      (f)(WiFi.BSSID(i), static_cast<uint8_t>(WiFi.channel(i)));                                   \
    }                                                                                              \
  } while (false)

#define DELETE_APS                                                                                 \
  do {                                                                                             \
    bool hasOtherSsid = false;                                                                     \
    int nNetworks = WiFi.scanComplete();                                                           \
    for (uint8_t i = 0; static_cast<int>(i) < nNetworks; ++i) {                                    \
      if (WiFi.SSID(i) == m_ssid) {                                                                \
        continue;                                                                                  \
      }                                                                                            \
      hasOtherSsid = true;                                                                         \
      break;                                                                                       \
    }                                                                                              \
    if (!hasOtherSsid) {                                                                           \
      WiFi.scanDelete();                                                                           \
    }                                                                                              \
  } while (false)

#endif
{
  m_isScanning = false;
  m_nextScan = millis() + m_scanFreq;
#ifdef ARDUINO_ARCH_ESP8266
  if (status != 0) {
    return;
  }
#endif

  LOG("processScan()");

  const int MAX_PEERS = 20;
  WifiEspNowPeerInfo oldPeers[MAX_PEERS];
  int nOldPeers = std::min(WifiEspNow.listPeers(oldPeers, MAX_PEERS), MAX_PEERS);
  const uint8_t PEER_FOUND = 0xFF; // assigned to .channel to indicate peer is matched

  FOREACH_AP([&](const uint8_t* bssid, uint8_t channel) {
    for (int i = 0; i < nOldPeers; ++i) {
      WifiEspNowPeerInfo* p = &oldPeers[i];
      if (std::equal(p->mac, p->mac + WIFIESPNOW_ALEN, bssid)) {
        p->channel = PEER_FOUND;
        break;
      }
    }
  });

  for (int i = 0; i < nOldPeers; ++i) {
    WifiEspNowPeerInfo* p = &oldPeers[i];
    if (p->channel == PEER_FOUND) {
      continue;
    }
    LOG("processScan removePeer(%02x:%02x:%02x:%02x:%02x:%02x)", p->mac[0], p->mac[1], p->mac[2],
        p->mac[3], p->mac[4], p->mac[5]);
    WifiEspNow.removePeer(p->mac);
  }

  FOREACH_AP([&](const uint8_t* mac, uint8_t channel) {
    LOG("processScan addPeer(%02x:%02x:%02x:%02x:%02x:%02x)", mac[0], mac[1], mac[2], mac[3],
        mac[4], mac[5]);
    WifiEspNow.addPeer(mac, channel, m_hasPeerKey ? m_peerKey : nullptr);
  });

  DELETE_APS;
}
