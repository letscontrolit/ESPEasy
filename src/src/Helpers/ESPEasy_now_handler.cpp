#include "ESPEasy_now_handler.h"

#ifdef USES_ESPEASY_NOW

# include "ESPEasy_time_calc.h"
# include "../DataStructs/ESPEasy_Now_packet.h"
# include "../Globals/SecuritySettings.h"
# include "../../ESPEasy_fdwdecl.h"
# include "../../ESPEasy_Log.h"

# include <list>

std::list<ESPEasy_Now_packet> ESPEasy_now_in_queue;

void ICACHE_FLASH_ATTR ESPEasy_now_onReceive(const uint8_t mac[6], const uint8_t *buf, size_t count, void *cbarg) {
  ESPEasy_now_in_queue.emplace_back(mac, buf, count);
}

bool ESPEasy_now_handler_t::begin()
{
  if (!WifiEspNow.begin()) { return false; }

  for (byte peer = 0; peer < ESPEASY_NOW_PEER_MAX; ++peer) {
    if (SecuritySettings.peerMacSet(peer)) {
      if (!WifiEspNow.addPeer(SecuritySettings.EspEasyNowPeerMAC[peer])) {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log;
          log.reserve(48);
          log  = F("ESPEasy_Now: Failed to add peer ");
          log += formatMAC(SecuritySettings.EspEasyNowPeerMAC[peer]);
          addLog(LOG_LEVEL_ERROR, log);
        }
      }
    }
  }

  // FIXME TD-er: Must check in settings if enabled
  WifiEspNow.onReceive(ESPEasy_now_onReceive, nullptr);

  use_EspEasy_now = true;
  return true;
}

void ESPEasy_now_handler_t::end()
{
  use_EspEasy_now = false;
  WifiEspNow.end();
}

bool ESPEasy_now_handler_t::loop()
{
  if (!ESPEasy_now_in_queue.empty()) {
    bool validPacket    = ESPEasy_now_in_queue.front().getHeader().checksumValid();
    const byte loglevel = validPacket ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR;

    if (loglevelActiveFor(loglevel)) {
      String log = F("ESPEasyNow: Message from ");
      log += formatMAC(ESPEasy_now_in_queue.front()._mac);

      if (!validPacket) {
        log += F(" INVALID CHECKSUM!");
      }
      addLog(loglevel, log);
    }

    if (!validPacket) {
      ESPEasy_now_in_queue.pop_front();
      return false;
    }

  # ifdef USES_MQTT

    // FIXME TD-er: Quick hack to just echo all data to the first enabled MQTT controller

    controllerIndex_t controllerIndex = firstEnabledMQTT_ControllerIndex();

    if (validControllerIndex(controllerIndex)) {
      String topic   = ESPEasy_now_in_queue.front().getString(0);
      String payload = ESPEasy_now_in_queue.front().getString(topic.length());

      MakeControllerSettings(ControllerSettings);
      LoadControllerSettings(controllerIndex, ControllerSettings);
      MQTTpublish(controllerIndex, topic.c_str(), payload.c_str(), ControllerSettings.mqtt_retainFlag());
    }

  # endif // ifdef USES_MQTT

    // FIXME TD-er: What to do when publish fails?
    ESPEasy_now_in_queue.pop_front();
    return true;
  }
  return false;
}

void ESPEasy_now_handler_t::sendDiscoveryAnnounce(byte channel)
{}

bool ESPEasy_now_handler_t::sendToMQTT(controllerIndex_t controllerIndex, const String& topic, const String& payload)
{
  if (!use_EspEasy_now) { return false; }


  MakeControllerSettings(ControllerSettings);
  LoadControllerSettings(controllerIndex, ControllerSettings);

  bool processed = false;

  if (ControllerSettings.enableESPEasyNowFallback() /*&& !WiFiConnected(10) */) {
    const size_t topic_length   = topic.length();
    const size_t payload_length = payload.length();

    // Todo: Add   cpluginID_t cpluginID; to the message
    size_t len = topic_length + payload_length + 1;
    
    ESPEasy_now_hdr header(ESPEasy_now_hdr::message_t::MQTTControllerMessage);
    ESPEasy_Now_packet msg(header, len);

    size_t pos = 0;
    pos += msg.addString(topic);
    pos += msg.addString(payload, pos);

    for (byte peer = 0; peer < ESPEASY_NOW_PEER_MAX && !processed; ++peer) {
      // FIXME TD-er: This must be optimized to keep the last working index.
      // Or else it may take quite a while to send each message
      if (SecuritySettings.peerMacSet(peer)) {
        if (WifiEspNow.send(SecuritySettings.EspEasyNowPeerMAC[peer], msg[0], msg.getSize())) {
          unsigned long timer             = millis() + 500;
          WifiEspNowSendStatus sendStatus = WifiEspNow.getSendStatus();

          while (!timeOutReached(timer) && sendStatus == WifiEspNowSendStatus::NONE) {
            sendStatus = WifiEspNow.getSendStatus();
            delay(1);
          }

          if (sendStatus == WifiEspNowSendStatus::OK) {
            processed = true;

            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              String log = F("ESPEasy Now: Sent via ESP-NOW to: ");
              log += formatMAC(SecuritySettings.EspEasyNowPeerMAC[peer]);
              addLog(LOG_LEVEL_INFO, log);
            }
          }
        } else {
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("ESPEasy Now: Sent via ESP-NOW failed to: ");
            log += formatMAC(SecuritySettings.EspEasyNowPeerMAC[peer]);
            addLog(LOG_LEVEL_INFO, log);
          }
        }
      }
    }
    
  }
  return processed;
}

#endif // ifdef USES_ESPEASY_NOW
