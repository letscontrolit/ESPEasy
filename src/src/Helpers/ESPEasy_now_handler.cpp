#include "ESPEasy_now_handler.h"

#ifdef USES_ESPEASY_NOW

# include "ESPEasy_time_calc.h"
# include "../DataStructs/ESPEasy_Now_packet.h"
# include "../Globals/SecuritySettings.h"
# include "../Globals/Settings.h"
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

  sendDiscoveryAnnounce();

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
      } else {
        log += F(" (");
        log += ESPEasy_now_in_queue.front().getHeader().cur_message_nr + 1;
        log += '/';
        log += ESPEasy_now_in_queue.front().getHeader().last_message_nr + 1;
        log += ')';
      }
      addLog(loglevel, log);
    }

    if (!validPacket) {
      ESPEasy_now_in_queue.pop_front();
      return false;
    }

    bool handled = false;

    switch (ESPEasy_now_in_queue.front().getHeader().message_type)
    {
      case ESPEasy_now_hdr::message_t::NotSet:
      case ESPEasy_now_hdr::message_t::ChecksumError:
        break;
      case ESPEasy_now_hdr::message_t::Acknowledgement:
        break;
      case ESPEasy_now_hdr::message_t::Announcement:
        handled = handle_DiscoveryAnnounce(ESPEasy_now_in_queue.front());
        break;
      case ESPEasy_now_hdr::message_t::MQTTControllerMessage:
        handled = handle_MQTTControllerMessage(ESPEasy_now_in_queue.front());
        break;
    }

    // FIXME TD-er: What to do when packet is not handled?
    ESPEasy_now_in_queue.pop_front();
    return handled;
  }
  return false;
}

void ESPEasy_now_handler_t::sendDiscoveryAnnounce(byte channel)
{
  String hostname = Settings.getHostname();
  size_t len      = hostname.length();
  ESPEasy_now_hdr header(ESPEasy_now_hdr::message_t::Announcement);
  ESPEasy_Now_packet msg(header, len);

  msg.setBroadcast();

  size_t pos = 0;
  msg.addString(hostname, pos);

  send(msg);
}

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
    msg.addString(topic,   pos);
    msg.addString(payload, pos);

    for (byte peer = 0; peer < ESPEASY_NOW_PEER_MAX && !processed; ++peer) {
      // FIXME TD-er: This must be optimized to keep the last working index.
      // Or else it may take quite a while to send each message
      if (SecuritySettings.peerMacSet(peer)) {
        msg.setMac(SecuritySettings.EspEasyNowPeerMAC[peer]);
        WifiEspNowSendStatus sendStatus = send(msg, millis() + ControllerSettings.ClientTimeout);

        switch (sendStatus) {
          case WifiEspNowSendStatus::OK:
          {
            processed = true;
            break;
          }
          default: break;
        }
      }
    }
  }
  return processed;
}

bool ESPEasy_now_handler_t::getPeerInfo(const uint8_t             *mac,
                                        ESPEasy_Now_peerInfo_meta& meta) const
{
  return _peerInfoMap.getPeer(mac, meta);
}

bool ESPEasy_now_handler_t::send(const ESPEasy_Now_packet& packet) {
  bool success = WifiEspNow.send(packet._mac, packet[0], packet.getSize());

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;

    if (success) {
      log = F("ESPEasy Now: Sent to: ");
    } else {
      log = F("ESPEasy Now: Sent FAILED to: ");
    }
    log += formatMAC(packet._mac);
    addLog(LOG_LEVEL_INFO, log);
  }
  return success;
}

WifiEspNowSendStatus ESPEasy_now_handler_t::send(const ESPEasy_Now_packet& packet, size_t timeout)
{
  if (!send(packet)) {
    return WifiEspNowSendStatus::NONE;
  }
  WifiEspNowSendStatus sendStatus = waitForSendStatus(timeout);

  if (sendStatus == WifiEspNowSendStatus::NONE) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("ESPEasy Now: TIMEOUT to: ");
      log += formatMAC(packet._mac);
      addLog(LOG_LEVEL_INFO, log);
    }
  }
  return sendStatus;
}

WifiEspNowSendStatus ESPEasy_now_handler_t::waitForSendStatus(size_t timeout) const
{
  WifiEspNowSendStatus sendStatus = WifiEspNowSendStatus::NONE;

  while (!timeOutReached(timeout) && sendStatus == WifiEspNowSendStatus::NONE) {
    sendStatus = WifiEspNow.getSendStatus();
    delay(1);
  }
  return sendStatus;
}

bool ESPEasy_now_handler_t::handle_DiscoveryAnnounce(const ESPEasy_Now_packet& packet)
{
  size_t payload_pos = 0;
  ESPEasy_Now_peerInfo_meta meta;

  meta.nodeName = packet.getString(payload_pos);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    size_t payloadSize = packet.getPayloadSize();
    log.reserve(payloadSize + 40);
    log  = F("ESPEasy Now discovery: ");
    log += formatMAC(packet._mac);
    log += '\n';
    log += meta.nodeName;

    while (payload_pos < payloadSize) {
      log += '\n';
      log += packet.getString(payload_pos);
    }
    addLog(LOG_LEVEL_INFO, log);
  }
  _peerInfoMap.addPeer(packet._mac, meta);
  return true;
}

bool ESPEasy_now_handler_t::handle_MQTTControllerMessage(const ESPEasy_Now_packet& packet)
{
  # ifdef USES_MQTT

  // FIXME TD-er: Quick hack to just echo all data to the first enabled MQTT controller

  controllerIndex_t controllerIndex = firstEnabledMQTT_ControllerIndex();

  if (validControllerIndex(controllerIndex)) {
    size_t pos     = 0;
    String topic   = packet.getString(pos);
    String payload = packet.getString(pos);

    MakeControllerSettings(ControllerSettings);
    LoadControllerSettings(controllerIndex, ControllerSettings);
    return MQTTpublish(controllerIndex, topic.c_str(), payload.c_str(), ControllerSettings.mqtt_retainFlag());
  }

  # endif // ifdef USES_MQTT
  return false;
}

#endif // ifdef USES_ESPEASY_NOW
