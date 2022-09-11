#include "WifiEspNow.h"

#include <string.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <c_types.h>
#include <espnow.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <esp_now.h>
#else
#error "This library supports ESP8266 and ESP32 only."
#endif

WifiEspNowClass WifiEspNow;

bool
WifiEspNowClass::begin()
{
  end();
  m_ready =
    esp_now_init() == 0 &&
#ifdef ARDUINO_ARCH_ESP8266
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO) == 0 &&
#endif
    esp_now_register_recv_cb(reinterpret_cast<esp_now_recv_cb_t>(WifiEspNowClass::rx)) == 0 &&
    esp_now_register_send_cb(reinterpret_cast<esp_now_send_cb_t>(WifiEspNowClass::tx)) == 0;
  return m_ready;
}

void
WifiEspNowClass::end()
{
  if (!m_ready) {
    return;
  }
  esp_now_deinit();
  m_ready = false;
}

bool
WifiEspNowClass::setPrimaryKey(const uint8_t key[WIFIESPNOW_KEYLEN])
{
  return m_ready && key != nullptr &&
#if defined(ARDUINO_ARCH_ESP8266)
         esp_now_set_kok(const_cast<u8*>(key), WIFIESPNOW_KEYLEN) == 0;
#elif defined(ARDUINO_ARCH_ESP32)
         esp_now_set_pmk(key) == ESP_OK;
#endif
}

int
WifiEspNowClass::listPeers(WifiEspNowPeerInfo* peers, int maxPeers) const
{
  if (!m_ready) {
    return 0;
  }
  int n = 0;
#if defined(ARDUINO_ARCH_ESP8266)
  for (u8* mac = esp_now_fetch_peer(true); mac != nullptr; mac = esp_now_fetch_peer(false)) {
    uint8_t channel = static_cast<uint8_t>(esp_now_get_peer_channel(mac));
#elif defined(ARDUINO_ARCH_ESP32)
  esp_now_peer_info_t peer;
  for (esp_err_t e = esp_now_fetch_peer(true, &peer); e == ESP_OK;
       e = esp_now_fetch_peer(false, &peer)) {
    uint8_t* mac = peer.peer_addr;
    uint8_t channel = peer.channel;
#endif
    if (n < maxPeers) {
      memcpy(peers[n].mac, mac, 6);
      peers[n].channel = channel;
    }
    ++n;
  }
  return n;
}

bool
WifiEspNowClass::hasPeer(const uint8_t mac[WIFIESPNOW_ALEN]) const
{
  return m_ready &&
#if defined(ARDUINO_ARCH_ESP8266)
         esp_now_is_peer_exist(const_cast<u8*>(mac)) > 0;
#elif defined(ARDUINO_ARCH_ESP32)
         esp_now_is_peer_exist(mac);
#endif
}

#if defined(ARDUINO_ARCH_ESP8266)
bool
WifiEspNowClass::addPeer(const uint8_t mac[WIFIESPNOW_ALEN], int channel,
                         const uint8_t key[WIFIESPNOW_KEYLEN])
{
  if (!m_ready) {
    return false;
  }

  if (this->hasPeer(mac)) {
    return esp_now_set_peer_channel(const_cast<u8*>(mac), static_cast<u8>(channel)) == 0 &&
           esp_now_set_peer_key(const_cast<u8*>(mac), const_cast<u8*>(key),
                                key == nullptr ? 0 : WIFIESPNOW_KEYLEN) == 0;
  }
  return esp_now_add_peer(const_cast<u8*>(mac), ESP_NOW_ROLE_SLAVE, static_cast<u8>(channel),
                          const_cast<u8*>(key), key == nullptr ? 0 : WIFIESPNOW_KEYLEN) == 0;
}
#elif defined(ARDUINO_ARCH_ESP32)
bool
WifiEspNowClass::addPeer(const uint8_t mac[WIFIESPNOW_ALEN], int channel,
                         const uint8_t key[WIFIESPNOW_KEYLEN], int netif)
{
  if (!m_ready) {
    return false;
  }

  esp_now_peer_info_t pi{};
  static_assert(WIFIESPNOW_ALEN == sizeof(pi.peer_addr), "");
  std::copy_n(mac, WIFIESPNOW_ALEN, pi.peer_addr);
  if (key != nullptr) {
    static_assert(WIFIESPNOW_KEYLEN == sizeof(pi.lmk), "");
    std::copy_n(key, WIFIESPNOW_KEYLEN, pi.lmk);
    pi.encrypt = true;
  }
  pi.channel = static_cast<uint8_t>(channel);
  pi.ifidx = static_cast<wifi_interface_t>(netif);

  if (hasPeer(mac)) {
    return esp_now_mod_peer(&pi) == ESP_OK;
  }
  return esp_now_add_peer(&pi) == ESP_OK;
}
#endif

bool
WifiEspNowClass::removePeer(const uint8_t mac[WIFIESPNOW_ALEN])
{
  return m_ready && esp_now_del_peer(const_cast<uint8_t*>(mac)) == 0;
}

void
WifiEspNowClass::onReceive(RxCallback cb, void* arg)
{
  m_rxCb = cb;
  m_rxArg = arg;
}

bool
WifiEspNowClass::send(const uint8_t mac[WIFIESPNOW_ALEN], const uint8_t* buf, size_t count)
{
  if (!m_ready || count > WIFIESPNOW_MAXMSGLEN || count == 0) {
    return false;
  }
  WifiEspNow.m_txRes = WifiEspNowSendStatus::NONE;
  return esp_now_send(const_cast<uint8_t*>(mac), const_cast<uint8_t*>(buf),
                      static_cast<int>(count)) == 0;
}

void
WifiEspNowClass::rx(const uint8_t* mac, const uint8_t* data, uint8_t len)
{
  if (WifiEspNow.m_rxCb != nullptr) {
    (*WifiEspNow.m_rxCb)(mac, data, len, WifiEspNow.m_rxArg);
  }
}

void
WifiEspNowClass::tx(const uint8_t* mac, uint8_t status)
{
  WifiEspNow.m_txRes = status == 0 ? WifiEspNowSendStatus::OK : WifiEspNowSendStatus::FAIL;
}
