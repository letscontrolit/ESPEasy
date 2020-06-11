#include "ESPEasy_now_handler.h"

#ifdef USES_ESPEASY_NOW

# include "ESPEasy_time_calc.h"
# include "../DataStructs/ESPEasy_Now_DuplicateCheck.h"
# include "../DataStructs/ESPEasy_Now_packet.h"
# include "../DataStructs/ESPEasy_now_splitter.h"
# include "../DataStructs/NodeStruct.h"
# include "../DataStructs/TimingStats.h"
# include "../Globals/ESPEasy_time.h"
# include "../Globals/Nodes.h"
# include "../Globals/SecuritySettings.h"
# include "../Globals/SendData_DuplicateChecker.h"
# include "../Globals/Settings.h"
# include "../../ESPEasy_fdwdecl.h"
# include "../../ESPEasy_Log.h"

# include <list>


# define ESPEASY_NOW_ACTIVVITY_TIMEOUT 60000 // 1 minute

# define ESPEASY_NOW_TMP_SSID       "ESPEASY_NOW"
# define ESPEASY_NOW_TMP_PASSPHRASE "random_passphrase"

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
  START_TIMER;
  ESPEasy_now_hdr header;
  memcpy(&header, buf, sizeof(ESPEasy_now_hdr));

  if (header.header_version != ESPEASY_NOW_HEADER_VERSION) {
    return;
  }

  size_t payload_length  = count - sizeof(ESPEasy_now_hdr);
  const uint8_t *payload = buf + sizeof(ESPEasy_now_hdr);

  const uint16_t checksum = calc_CRC16(reinterpret_cast<const char *>(payload), payload_length);

  if (header.checksum != checksum) {
    return;
  }
  uint64_t key = mac_to_key(mac, header.message_type, header.message_count);
  ESPEasy_now_in_queue[key].addPacket(header.packet_nr, mac, buf, count);
  STOP_TIMER(RECEIVE_ESPEASY_NOW_LOOP);
}

bool ESPEasy_now_handler_t::begin()
{
  if (!Settings.UseESPEasyNow()) { return false; }

  if (use_EspEasy_now) {
    return true;
  }

  _last_used = millis();
  int channel = WiFi.channel();
  MAC_address bssid;

  if (espeasy_now_only) {
    WifiScan(false, false);
    addPeerFromWiFiScan();
  }

  const NodeStruct *preferred = Nodes.getPreferredNode();

  if (preferred != nullptr) {
    channel = preferred->channel;
    bssid.set(preferred->ap_mac);
  }

  const String ssid       = F(ESPEASY_NOW_TMP_SSID);
  const String passphrase = F(ESPEASY_NOW_TMP_PASSPHRASE);

  if (espeasy_now_only) {
    if (bssid.all_zero()) {
      WiFi.begin(getLastWiFiSettingsSSID(), getLastWiFiSettingsPassphrase(), channel);
    } else {
      WiFi.begin(getLastWiFiSettingsSSID(), getLastWiFiSettingsPassphrase(), channel, bssid.mac);
    }
  }
  setAP(true);

  int ssid_hidden    = 1;
  int max_connection = 6;
  WiFi.softAP(ssid.c_str(), passphrase.c_str(), channel, ssid_hidden, max_connection);

  //    WiFi.softAPdisconnect(false);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("ESPEasy-Now: begin on channel ");
    log += channel;
    addLog(LOG_LEVEL_INFO, log);
  }

  if (!WifiEspNow.begin()) {
    addLog(LOG_LEVEL_ERROR, F("ESPEasy-Now: Failed to initialize ESPEasy-Now"));
    return false;
  }

  for (byte peer = 0; peer < ESPEASY_NOW_PEER_MAX; ++peer) {
    if (SecuritySettings.peerMacSet(peer)) {
      add_peer(SecuritySettings.EspEasyNowPeerMAC[peer], 0);
    }
  }

  for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
    if (it->second.ESPEasyNowPeer) {
      add_peer(it->second.ESPEasy_Now_MAC(), it->second.channel);
    }
  }


  // FIXME TD-er: Must check in settings if enabled
  WifiEspNow.onReceive(ESPEasy_now_onReceive, nullptr);

  sendDiscoveryAnnounce();

  use_EspEasy_now = true;
  _last_used      = 0;
  addLog(LOG_LEVEL_INFO, F("ESPEasy-Now enabled"));
  return true;
}

void ESPEasy_now_handler_t::end()
{
  use_EspEasy_now = false;
  WifiEspNow.end();
  addLog(LOG_LEVEL_INFO, F("ESPEasy-Now disabled"));
}

bool ESPEasy_now_handler_t::loop()
{
  if (temp_disable_EspEasy_now_timer != 0) {
    if (timeOutReached(temp_disable_EspEasy_now_timer)) {
      if (begin()) {
        temp_disable_EspEasy_now_timer = 0;
      }
    } else {
      if (use_EspEasy_now) {
        end();
      }
      return false;
    }
  } else {
    if (Settings.UseESPEasyNow() != use_EspEasy_now) {
      if (!use_EspEasy_now) {
        begin();
      } else {
        end();
      }
    }
  }
  bool somethingProcessed = false;

  if (!ESPEasy_now_in_queue.empty()) {
    for (auto it = ESPEasy_now_in_queue.begin(); it != ESPEasy_now_in_queue.end();) {
      bool removeMessage = true;
      START_TIMER;

      if (!it->second.messageComplete()) {
        if (it->second.expired()) {
          STOP_TIMER(EXPIRED_ESPEASY_NOW_LOOP);

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
        bool mustKeep = !removeMessage;
        somethingProcessed = processMessage(it->second, mustKeep);
        removeMessage      = !mustKeep;
        STOP_TIMER(HANDLE_ESPEASY_NOW_LOOP);
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

  if (_send_failed_count > 30 /*|| !active()*/) {
    _send_failed_count = 0;
    espeasy_now_only   = true;

    // Start scanning the next channel to see if we may end up with a new found node
    //    WifiScan(false, false);
    //    addPeerFromWiFiScan();
    //    _last_used = millis();
    begin();
  }
  return somethingProcessed;
}

bool ESPEasy_now_handler_t::active() const
{
  if (!use_EspEasy_now) {
    return false;
  }

  if (_last_used == 0) {
    return false;
  }
  return timePassedSince(_last_used) < ESPEASY_NOW_ACTIVVITY_TIMEOUT;
}

void ESPEasy_now_handler_t::addPeerFromWiFiScan()
{
  const int8_t scanCompleteStatus = WiFi.scanComplete();

  for (int8_t i = 0; i < scanCompleteStatus; ++i) {
    addPeerFromWiFiScan(i);
  }
}

void ESPEasy_now_handler_t::addPeerFromWiFiScan(uint8_t scanIndex)
{
  int8_t scanCompleteStatus = WiFi.scanComplete();

  switch (scanCompleteStatus) {
    case 0:  // Nothing (yet) found
      return;
    case -1: // WIFI_SCAN_RUNNING
      return;
    case -2: // WIFI_SCAN_FAILED
      return;
  }

  if (scanIndex > scanCompleteStatus) { return; }
  MAC_address peer_mac = WiFi.BSSID(scanIndex);
  auto nodeInfo        = Nodes.getNodeByMac(peer_mac);

  if (nodeInfo != nullptr) {
    nodeInfo->setRSSI(WiFi.RSSI(scanIndex));
    nodeInfo->channel = WiFi.channel(scanIndex);
  } else {
    // FIXME TD-er: For now we assume the other node uses AP for ESPEasy-now
    NodeStruct tmpNodeInfo;
    tmpNodeInfo.setRSSI(WiFi.RSSI(scanIndex));
    tmpNodeInfo.channel = WiFi.channel(scanIndex);
    peer_mac.get(tmpNodeInfo.ap_mac);
    tmpNodeInfo.setESPEasyNow_mac(peer_mac);

    if (tmpNodeInfo.markedAsPriorityPeer()) {
      Nodes.addNode(tmpNodeInfo);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("ESPEasy-Now: Found node via WiFi scan: ");
        log += peer_mac.toString();
        log += F(" ");
        log += tmpNodeInfo.getRSSI();
        log += F(" dBm ch: ");
        log += tmpNodeInfo.channel;
        addLog(LOG_LEVEL_INFO, log);
      }

      // Must trigger a discovery request from the node.
      // FIXME TD-er: Disable auto discovery for now
      //    sendDiscoveryAnnounce(peer_mac, WiFi.channel(scanIndex));
    }
  }
}

bool ESPEasy_now_handler_t::processMessage(const ESPEasy_now_merger& message, bool& mustKeep)
{
  addLog(LOG_LEVEL_INFO, message.getLogString());
  bool handled = false;
  mustKeep = true;

  switch (message.getFirstHeader().message_type)
  {
    case ESPEasy_now_hdr::message_t::NotSet:
    case ESPEasy_now_hdr::message_t::ChecksumError:
      break;
    case ESPEasy_now_hdr::message_t::Acknowledgement:
      break;
    case ESPEasy_now_hdr::message_t::Announcement:
      handled = handle_DiscoveryAnnounce(message, mustKeep);
      break;
    case ESPEasy_now_hdr::message_t::NTP_Query:
      handled = handle_NTPquery(message, mustKeep);
      break;
    case ESPEasy_now_hdr::message_t::MQTTControllerMessage:
      handled = handle_MQTTControllerMessage(message, mustKeep);
      break;
    case ESPEasy_now_hdr::message_t::MQTTCheckControllerQueue:
      handled = handle_MQTTCheckControllerQueue(message, mustKeep);
      break;
    case ESPEasy_now_hdr::message_t::SendData_DuplicateCheck:
      handled = handle_SendData_DuplicateCheck(message, mustKeep);
      break;
  }

  if (handled) {
    _last_used = millis();
  }

  return handled;
}

// *************************************************************
// * Discovery Announcement
// *************************************************************

void ESPEasy_now_handler_t::sendDiscoveryAnnounce()
{
  MAC_address broadcast;

  for (int i = 0; i < 6; ++i) {
    broadcast.mac[i] = 0xFF;
  }
  sendDiscoveryAnnounce(broadcast);
}

void ESPEasy_now_handler_t::sendDiscoveryAnnounce(const MAC_address& mac, int channel)
{
  const NodeStruct *thisNode = Nodes.getThisNode();

  if (thisNode == nullptr) {
    // Should not happen
    return;
  }
  size_t len = sizeof(NodeStruct);
  ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::Announcement, len);
  msg.addBinaryData(reinterpret_cast<const uint8_t *>(thisNode), len);
  msg.send(mac, channel);
}

bool ESPEasy_now_handler_t::handle_DiscoveryAnnounce(const ESPEasy_now_merger& message, bool& mustKeep)
{
  mustKeep = false;
  NodeStruct received;

  // Discovery messages have a single binary blob, starting at 0
  size_t payload_pos = 0;

  message.getBinaryData(reinterpret_cast<uint8_t *>(&received), sizeof(NodeStruct), payload_pos);

  if (!received.validate()) { return false; }

  MAC_address mac;
  message.getMac(mac);

  if (!received.setESPEasyNow_mac(mac)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log;
      log  = F("ESPEasy Now: Received discovery message from MAC not stated in message: ");
      log += mac.toString();
      addLog(LOG_LEVEL_ERROR, log);
    }
    return false;
  }

  // FIXME TD-er: Disable auto discovery for now
  //  bool isNewNode = Nodes.getNodeByMac(mac) == nullptr;

  Nodes.addNode(received);

  // Test to see if the discovery announce could be a good candidate for next NTP query.
  _best_NTP_candidate.find_best_NTP(
    mac,
    static_cast<timeSource_t>(received.timeSource),
    received.lastUpdated);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    size_t payloadSize = message.getPayloadSize();
    log.reserve(payloadSize + 40);
    log  = F("ESPEasy Now discovery: ");
    log += message.getLogString();
    log += '\n';
    log += received.getSummary();
    addLog(LOG_LEVEL_INFO, log);
  }

  // FIXME TD-er: Disable auto discovery for now

  /*
     if (isNewNode) {
      sendDiscoveryAnnounce(mac);
     }
   */
  return true;
}

// *************************************************************
// * NTP Query
// *************************************************************

void ESPEasy_now_handler_t::sendNTPquery()
{
  if (!_best_NTP_candidate.hasLowerWander()) { return; }
  MAC_address mac;

  if (!_best_NTP_candidate.getMac(mac)) { return; }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("ESPEasy-Now: Send NTP query to: ");
    log += mac.toString();
    addLog(LOG_LEVEL_INFO, log);
  }

  _best_NTP_candidate.markSendTime();
  ESPEasy_Now_NTP_query query;
  size_t len = sizeof(ESPEasy_Now_NTP_query);
  ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::NTP_Query, len);
  msg.addBinaryData(reinterpret_cast<uint8_t *>(&query), len);
  msg.send(mac.mac);
}

void ESPEasy_now_handler_t::sendNTPbroadcast()
{
  ESPEasy_Now_NTP_query query;

  query.createBroadcastNTP();
  size_t len = sizeof(ESPEasy_Now_NTP_query);
  ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::NTP_Query, len);
  msg.addBinaryData(reinterpret_cast<uint8_t *>(&query), len);
  msg.send(query._mac);
}

bool ESPEasy_now_handler_t::handle_NTPquery(const ESPEasy_now_merger& message, bool& mustKeep)
{
  mustKeep = false;
  ESPEasy_Now_NTP_query query;

  // NTP query messages have a single binary blob, starting at 0
  size_t payload_pos = 0;

  message.getBinaryData(reinterpret_cast<uint8_t *>(&query), sizeof(ESPEasy_Now_NTP_query), payload_pos);

  if (query._timeSource == timeSource_t::No_time_source) {
    // Received a query, must generate an answer for it.

    // first fetch the MAC address to reply to
    if (!message.getMac(query._mac)) { return false; }

    // Fill the reply
    query.createReply(message.getFirstPacketTimestamp());

    size_t len = sizeof(ESPEasy_Now_NTP_query);
    ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::NTP_Query, len);
    msg.addBinaryData(reinterpret_cast<uint8_t *>(&query), len);
    msg.send(query._mac);
    return true;
  }

  // Received a reply on our own query
  return _best_NTP_candidate.processReply(query, message.getFirstPacketTimestamp());
}

// *************************************************************
// * MQTT controller forwarder
// *************************************************************

bool ESPEasy_now_handler_t::sendToMQTT(controllerIndex_t controllerIndex, const String& topic, const String& payload)
{
  if (!use_EspEasy_now) { return false; }

  const uint8_t distance = Nodes.getDistance();

  if ((distance == 0) || (distance == 255)) {
    // No need to send via ESPEasy_Now.
    // We're either connected (distance == 0)
    // or have no neighbor that can forward the message (distance == 255)
    return false;
  }

  MakeControllerSettings(ControllerSettings);
  LoadControllerSettings(controllerIndex, ControllerSettings);

  bool processed = false;

  if (ControllerSettings.enableESPEasyNowFallback() /*&& !WiFiConnected(10) */) {
    const NodeStruct *preferred = Nodes.getPreferredNode();

    if (preferred != nullptr) {

      switch (_preferredNodeMQTTqueueState.state) {
        case ESPEasy_Now_MQTT_queue_check_packet::QueueState::Unset:
        case ESPEasy_Now_MQTT_queue_check_packet::QueueState::Full:
          sendMQTTCheckControllerQueue(controllerIndex);
          return false;
        case ESPEasy_Now_MQTT_queue_check_packet::QueueState::Empty:
        break;
      }


      // each string has null termination
      const size_t topic_length   = topic.length() + 1;
      const size_t payload_length = payload.length() + 1;

      // Todo: Add   cpluginID_t cpluginID; to the message
      size_t len = topic_length + payload_length;

      ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::MQTTControllerMessage, len);

      msg.addString(topic);
      msg.addString(payload);

      MAC_address mac                 = preferred->ESPEasy_Now_MAC();
      WifiEspNowSendStatus sendStatus = msg.send(mac, millis() + 2 * ControllerSettings.ClientTimeout, preferred->channel);

      switch (sendStatus) {
        case WifiEspNowSendStatus::OK:
        {
          processed = true;
          break;
        }
        case WifiEspNowSendStatus::NONE:
        case WifiEspNowSendStatus::FAIL:
        {
          ++_send_failed_count;
          break;
        }
        default: break;
      }
    }
  }
  return processed;
}

bool ESPEasy_now_handler_t::handle_MQTTControllerMessage(const ESPEasy_now_merger& message, bool& mustKeep)
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
    bool success = MQTTpublish(controllerIndex, topic.c_str(), payload.c_str(), ControllerSettings.mqtt_retainFlag());

    MAC_address mac;
    if (message.getMac(mac)) {
      ESPEasy_Now_MQTT_queue_check_packet query;
      query.setState(MQTT_queueFull(controllerIndex));
      sendMQTTCheckControllerQueue(mac, 0, query.state);
    }

    mustKeep = !success;
    return success;
  }

  # endif // ifdef USES_MQTT
  mustKeep = false;
  return false;
}

// *************************************************************
// * Check MQTT queue state of preferred node
// *************************************************************
bool ESPEasy_now_handler_t::sendMQTTCheckControllerQueue(controllerIndex_t controllerIndex)
{
  if (!use_EspEasy_now) { return false; }

  const uint8_t distance = Nodes.getDistance();

  if ((distance == 0) || (distance == 255)) {
    // No need to send via ESPEasy_Now.
    // We're either connected (distance == 0)
    // or have no neighbor that can forward the message (distance == 255)
    return false;
  }
  const NodeStruct *preferred = Nodes.getPreferredNode();

  if (preferred != nullptr) {
    return sendMQTTCheckControllerQueue(preferred->ESPEasy_Now_MAC(), preferred->channel);
  }
  return false;
}

bool ESPEasy_now_handler_t::sendMQTTCheckControllerQueue(const MAC_address& mac, int channel, ESPEasy_Now_MQTT_queue_check_packet::QueueState state) {
  ESPEasy_Now_MQTT_queue_check_packet query;
  query.state = state;
  size_t len = sizeof(ESPEasy_Now_MQTT_queue_check_packet);
  ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::MQTTCheckControllerQueue, len);
  msg.addBinaryData(reinterpret_cast<uint8_t *>(&query), len);
  size_t timeout = 10;
  WifiEspNowSendStatus sendStatus = msg.send(mac, timeout, channel);
  switch (sendStatus) {
    case WifiEspNowSendStatus::OK:
    {
      return true;
    }
    case WifiEspNowSendStatus::NONE:
    case WifiEspNowSendStatus::FAIL:
    {
      break;
    }
    default: break;
  }
  return false;
}


bool ESPEasy_now_handler_t::handle_MQTTCheckControllerQueue(const ESPEasy_now_merger& message, bool& mustKeep)
{
  mustKeep = false;
  # ifdef USES_MQTT

  ESPEasy_Now_MQTT_queue_check_packet query;
  size_t payload_pos = 0;
  message.getBinaryData(reinterpret_cast<uint8_t *>(&query), sizeof(ESPEasy_Now_NTP_query), payload_pos);

  controllerIndex_t controllerIndex = firstEnabledMQTT_ControllerIndex();

  if (validControllerIndex(controllerIndex)) {
    if (query.isSet()) {
      // Got an answer from our query
      _preferredNodeMQTTqueueState = query;
      return true;
    } else {
      MAC_address mac;
      if (message.getMac(mac)) { 
        // We have to give our own queue state and reply
        query.setState(MQTT_queueFull(controllerIndex));

        size_t len = sizeof(ESPEasy_Now_MQTT_queue_check_packet);
        ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::MQTTCheckControllerQueue, len);
        msg.addBinaryData(reinterpret_cast<uint8_t *>(&query), len);
        msg.send(mac);
        return true;
      }
    }
  }

  # endif // ifdef USES_MQTT
  return false;
}

// *************************************************************
// * Controller Message Duplicate Check
// *************************************************************

void ESPEasy_now_handler_t::sendSendData_DuplicateCheck(uint32_t                              key,
                                                        ESPEasy_Now_DuplicateCheck::message_t message_type,
                                                        const MAC_address                   & mac)
{
  ESPEasy_Now_DuplicateCheck check(key, message_type);
  size_t len = sizeof(ESPEasy_Now_DuplicateCheck);
  ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::SendData_DuplicateCheck, len);

  msg.addBinaryData(reinterpret_cast<uint8_t *>(&check), len);

  switch (message_type) {
    case ESPEasy_Now_DuplicateCheck::message_t::KeyToCheck:
      msg.sendToBroadcast();
      break;
    case ESPEasy_Now_DuplicateCheck::message_t::AlreadyProcessed:
      msg.send(mac);
      break;
  }

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    log = F("ESPEasy_now dup check: ");

    switch (message_type) {
      case ESPEasy_Now_DuplicateCheck::message_t::KeyToCheck:
        log += F("Broadcast check key ");
        log += key;
        break;
      case ESPEasy_Now_DuplicateCheck::message_t::AlreadyProcessed:
        log += F("Processed key ");
        log += key;
        log += ' ';
        log += mac.toString();
        break;
    }
    addLog(LOG_LEVEL_DEBUG, log);
  }
}

bool ESPEasy_now_handler_t::handle_SendData_DuplicateCheck(const ESPEasy_now_merger& message, bool& mustKeep)
{
  mustKeep = false;
  ESPEasy_Now_DuplicateCheck check;
  size_t payload_pos = 0;

  message.getBinaryData(reinterpret_cast<uint8_t *>(&check), sizeof(ESPEasy_Now_DuplicateCheck), payload_pos);

  switch (check._type) {
    case ESPEasy_Now_DuplicateCheck::message_t::KeyToCheck:

      // This is a query from another node.
      // Check if it has already been processed by some node.
      if (SendData_DuplicateChecker.historicKey(check._key)) {
        // Must reply back to that node we already have seen it
        MAC_address mac;

        if (message.getMac(mac)) {
          sendSendData_DuplicateCheck(check._key,
                                      ESPEasy_Now_DuplicateCheck::message_t::AlreadyProcessed,
                                      mac);
        }
      }
      return true;

    case ESPEasy_Now_DuplicateCheck::message_t::AlreadyProcessed:

      // This is a rejection indicating some other node already has the data processed
      SendData_DuplicateChecker.remove(check._key);

      return true;
  }
  return false;
}

bool ESPEasy_now_handler_t::add_peer(const MAC_address& mac, int channel) const
{
  MAC_address this_mac;

  WiFi.macAddress(this_mac.mac);

  // Don't add yourself as a peer
  if (this_mac == mac) { return false; }

  if (!WifiEspNow.addPeer(mac.mac, channel)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log;
      log.reserve(48);
      log  = F("ESPEasy_Now: Failed to add peer ");
      log += MAC_address(mac).toString();
      addLog(LOG_LEVEL_ERROR, log);
    }
    return false;
  }
  return true;
}

#endif // ifdef USES_ESPEASY_NOW
