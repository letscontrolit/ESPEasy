#include "ESPEasy_now_handler.h"

#ifdef USES_ESPEASY_NOW

# include "ESPEasy_time_calc.h"
# include "../DataStructs/ESPEasy_Now_packet.h"
# include "../DataStructs/NodeStruct.h"
# include "../Globals/Nodes.h"
# include "../Globals/SecuritySettings.h"
# include "../Globals/Settings.h"
# include "../../ESPEasy_fdwdecl.h"
# include "../../ESPEasy_Log.h"

# include <list>


static uint64_t mac_to_key(const uint8_t *mac, ESPEasy_now_hdr::message_t messageType, uint8_t message_count)
{
  uint64_t key = message_count;

  key  = key << 8;
  key += static_cast<uint8_t>(messageType);

  for (byte i = 0; i < 6; ++i) {
    key  = key << 8;
    key += mac[i];
  }
  return key;
}

std::map<uint64_t, ESPEasy_now_merger> ESPEasy_now_in_queue;

void ICACHE_FLASH_ATTR ESPEasy_now_onReceive(const uint8_t mac[6], const uint8_t *buf, size_t count, void *cbarg) {
  if (count < sizeof(ESPEasy_now_hdr)) {
    return; // Too small
  }
  ESPEasy_now_hdr header;
  memcpy(&header, buf, sizeof(ESPEasy_now_hdr));

  if (!header.checksumValid()) {
    return;
  }
  uint64_t key = mac_to_key(mac, header.message_type, header.message_count);
  ESPEasy_now_in_queue[key].addPacket(header.packet_nr, mac, buf, count);
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
  bool somethingProcessed = false;

  if (!ESPEasy_now_in_queue.empty()) {
    for (auto it = ESPEasy_now_in_queue.begin(); it != ESPEasy_now_in_queue.end();) {
      bool removeMessage = true;

      if (!it->second.messageComplete()) {
        if (it->second.expired()) {
          if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
            String log = it->second.getLogString();
            log += F(" Expired!");
            addLog(LOG_LEVEL_ERROR, log);
          }
        } else {
          removeMessage = false;
        }
      } else {
        // Process it
        somethingProcessed = processMessage(it->second);        
      }

      if (removeMessage) {
        it = ESPEasy_now_in_queue.erase(it);
        // FIXME TD-er: For now only process one item and then wait for the next loop.
        if (somethingProcessed) {
          return true;
        }
      } else {
        ++it;
      }
    }
  }
  return somethingProcessed;
}

void ESPEasy_now_handler_t::sendDiscoveryAnnounce(byte channel)
{
  NodeStruct thisNode;

  thisNode.setLocalData();
  size_t len = sizeof(NodeStruct);
  ESPEasy_now_hdr header(ESPEasy_now_hdr::message_t::Announcement);
  ESPEasy_Now_packet msg(header, len);
  msg.addBinaryData(reinterpret_cast<uint8_t *>(&thisNode), len);
  msg.setBroadcast();
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

bool ESPEasy_now_handler_t::processMessage(const ESPEasy_now_merger& message)
{

  addLog(LOG_LEVEL_INFO, message.getLogString());
  bool handled = false;

  switch (message.getFirstHeader().message_type)
  {
    case ESPEasy_now_hdr::message_t::NotSet:
    case ESPEasy_now_hdr::message_t::ChecksumError:
      break;
    case ESPEasy_now_hdr::message_t::Acknowledgement:
      break;
    case ESPEasy_now_hdr::message_t::Announcement:
      handled = handle_DiscoveryAnnounce(message);
      break;
    case ESPEasy_now_hdr::message_t::MQTTControllerMessage:
      handled = handle_MQTTControllerMessage(message);
      break;
  }

  return handled;
}


bool ESPEasy_now_handler_t::handle_DiscoveryAnnounce(const ESPEasy_now_merger& message)
{
  NodeStruct received;

  message.getBinaryData(reinterpret_cast<uint8_t *>(&received), sizeof(NodeStruct));
  Nodes.addNode(received);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    uint8_t mac[6] = {0};
    message.getMac(mac);
    size_t payloadSize = message.getPayloadSize();
    log.reserve(payloadSize + 40);
    log  = F("ESPEasy Now discovery: ");
    log += formatMAC(mac);
    log += '\n';
    log += received.getSummary();
    addLog(LOG_LEVEL_INFO, log);
  }
  return true;
}

bool ESPEasy_now_handler_t::handle_MQTTControllerMessage(const ESPEasy_now_merger& message)
{
  # ifdef USES_MQTT

  // FIXME TD-er: Quick hack to just echo all data to the first enabled MQTT controller

  controllerIndex_t controllerIndex = firstEnabledMQTT_ControllerIndex();

  if (validControllerIndex(controllerIndex)) {
    size_t pos     = 0;
    String topic   = message.getString(pos);
    String payload = message.getString(pos);

    MakeControllerSettings(ControllerSettings);
    LoadControllerSettings(controllerIndex, ControllerSettings);
    return MQTTpublish(controllerIndex, topic.c_str(), payload.c_str(), ControllerSettings.mqtt_retainFlag());
  }

  # endif // ifdef USES_MQTT
  return false;
}

#endif // ifdef USES_ESPEASY_NOW
