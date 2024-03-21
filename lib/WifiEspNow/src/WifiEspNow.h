/**
 * @mainpage WifiEspNow
 *
 * https://github.com/yoursunny/WifiEspNow
 */

#ifndef WIFIESPNOW_H
#define WIFIESPNOW_H

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif

#include <cstddef>
#include <cstdint>

/** @brief Address length. */
static const int WIFIESPNOW_ALEN = 6;

/** @brief Key length. */
static const int WIFIESPNOW_KEYLEN = 16;

/** @brief Maximum message length. */
static const int WIFIESPNOW_MAXMSGLEN = 250;

struct WifiEspNowPeerInfo {
  uint8_t mac[WIFIESPNOW_ALEN];
  uint8_t channel;
};

/** @brief Result of send operation. */
enum class WifiEspNowSendStatus : uint8_t {
  NONE = 0, ///< result unknown, send in progress
  OK   = 1, ///< unicast message acknowledged by peer; multicast message transmitted
  FAIL = 2, ///< sending failed
};

class WifiEspNowClass
{
public:
  /**
   * @brief Initialize ESP-NOW.
   * @return whether success.
   */
  bool
  begin();

  /** @brief Stop ESP-NOW. */
  void
  end();

  /**
   * @brief Set primary key, also known as KOK or PMK.
   * @param key primary encryption key.
   * @return whether success.
   */
  bool
  setPrimaryKey(const uint8_t key[WIFIESPNOW_KEYLEN]);

  /**
   * @brief List current peers.
   * @param[out] peers buffer for peer information.
   * @param maxPeers buffer size.
   * @return total number of peers, @c std::min(retval,maxPeers) is written to @p peers .
   */
  int
  listPeers(WifiEspNowPeerInfo* peers, int maxPeers) const;

  /**
   * @brief Test whether peer exists.
   * @param mac peer MAC address.
   * @return whether peer exists.
   */
  bool
  hasPeer(const uint8_t mac[WIFIESPNOW_ALEN]) const;

  /**
   * @brief Add a peer or change peer channel.
   * @param mac peer MAC address.
   * @param channel peer channel, 0 for current channel.
   * @param key encryption key, nullptr to disable encryption.
   * @param netif (ESP32 only) WiFi interface.
   * @return whether success.
   */
#if defined(ARDUINO_ARCH_ESP8266)
  bool
  addPeer(const uint8_t mac[WIFIESPNOW_ALEN], int channel = 0, const uint8_t key[WIFIESPNOW_KEYLEN] = nullptr);
#elif defined(ARDUINO_ARCH_ESP32)
  bool
  addPeer(const uint8_t mac[WIFIESPNOW_ALEN], int channel = 0, const uint8_t key[WIFIESPNOW_KEYLEN] = nullptr, int netif = ESP_IF_WIFI_AP);
#endif

  /**
   * @brief Remove a peer.
   * @param mac peer MAC address.
   * @return whether success.
   */
  bool
  removePeer(const uint8_t mac[WIFIESPNOW_ALEN]);

  using RxCallback = void (*)(const uint8_t mac[WIFIESPNOW_ALEN], const uint8_t* buf, size_t count, void* arg);

  /**
   * @brief Set receive callback.
   * @param cb the callback.
   * @param arg an arbitrary argument passed to the callback.
   * @note Only one callback is allowed; this replaces any previous callback.
   */
  void
  onReceive(RxCallback cb, void* arg);

  /**
   * @brief Send a message.
   * @param mac destination MAC address, nullptr for all peers.
   * @param buf payload.
   * @param count payload size, must not exceed @p WIFIESPNOW_MAXMSGLEN .
   * @return whether success (message queued for transmission).
   */
  bool
  send(const uint8_t mac[WIFIESPNOW_ALEN], const uint8_t* buf, size_t count);

  /** @brief Retrieve status of last sent message. */
  WifiEspNowSendStatus
  getSendStatus() const
  {
    return m_txRes;
  }

private:
  static void
  rx(const uint8_t* mac, const uint8_t* data, uint8_t len);

  static void
  tx(const uint8_t* mac, uint8_t status);

private:
  RxCallback m_rxCb = nullptr;
  void* m_rxArg = nullptr;
  WifiEspNowSendStatus m_txRes = WifiEspNowSendStatus::NONE;
  bool m_ready = false;
};

/** @brief ESP-NOW API. */
extern WifiEspNowClass WifiEspNow;

#endif // WIFIESPNOW_H
