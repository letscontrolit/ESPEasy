#ifndef WIFIESPNOW_BROADCAST_H
#define WIFIESPNOW_BROADCAST_H

#include "WifiEspNow.h"

#include <WString.h>

class WifiEspNowBroadcastClass
{
public:
  /**
   * @brief Initialize ESP-NOW with pseudo broadcast.
   * @param ssid AP SSID to announce and find peers.
   * @param channel AP channel, used if there is no STA connection.
   * @param scanFreq how often to scan for peers (milliseconds).
   * @return whether success.
   */
  bool
  begin(const char* ssid, int channel = 1, int scanFreq = 15000);

  /** @brief Stop ESP-NOW. */
  void
  end();

  /**
   * @brief Refresh peers if scanning is due.
   *
   * This should be invoked in Arduino sketch @c loop() function.
   */
  void
  loop();

  /**
   * @brief Set encryption keys.
   * @param primary primary key, also known as KOK or PMK.
   * @param peer peer key, also known as LMK; nullptr to disable encryption.
   *             The same peer key is applied to every discovered peer.
   * @return whether success.
   */
  bool
  setKey(const uint8_t primary[WIFIESPNOW_KEYLEN], const uint8_t peer[WIFIESPNOW_KEYLEN] = nullptr);

  /**
   * @brief Set receive callback.
   * @param cb the callback.
   * @param arg an arbitrary argument passed to the callback.
   * @note Only one callback is allowed; this replaces any previous callback.
   */
  void
  onReceive(WifiEspNowClass::RxCallback cb, void* arg)
  {
    WifiEspNow.onReceive(cb, arg);
  }

  /**
   * @brief Broadcast a message.
   * @param buf payload.
   * @param count payload size, must not exceed @c WIFIESPNOW_MAXMSGLEN .
   * @return whether success (message queued for transmission).
   */
  bool
  send(const uint8_t* buf, size_t count)
  {
    return WifiEspNow.send(nullptr, buf, count);
  }

private:
  void
  scan();

#if defined(ARDUINO_ARCH_ESP8266)
  static void
  processScan(void* result, int status);

  void
  processScan2(void* result, int status);
#elif defined(ARDUINO_ARCH_ESP32)
  void
  processScan();
#endif

private:
  String m_ssid;
  uint8_t m_peerKey[WIFIESPNOW_KEYLEN];
  int m_scanFreq = 0;
  unsigned long m_nextScan = 0;
  bool m_isScanning = false;
  bool m_hasPeerKey = false;
};

/**
 * @brief ESP-NOW pseudo broadcast.
 *
 * In pseudo broadcast mode, every node announces itself as a group member by advertising a
 * certain AP SSID. A node periodically scans other BSSIDs announcing the same SSID, and adds
 * them as ESP-NOW peers. Messages are sent to all known peers.
 *
 * Pseudo broadcast does not depend on ESP-NOW API to support broadcast.
 */
extern WifiEspNowBroadcastClass WifiEspNowBroadcast;

#endif // WIFIESPNOW_BROADCAST_H
