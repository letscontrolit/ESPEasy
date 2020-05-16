#include "ESPEasy_now_splitter.h"

#ifdef USES_ESPEASY_NOW

# include "../../ESPEasy_Log.h"
# include "../DataStructs/TimingStats.h"
# include "../Helpers/ESPEasy_time_calc.h"

static uint8_t ESPEasy_now_message_count = 1;

ESPEasy_now_splitter::ESPEasy_now_splitter(ESPEasy_now_hdr::message_t message_type, size_t totalSize)
  : _header(message_type), _totalSize(totalSize)
{
  _header.message_count = ++ESPEasy_now_message_count;
}

size_t ESPEasy_now_splitter::addBinaryData(const uint8_t *data, size_t length)
{
  size_t data_left = length;

  while ((data_left > 0) && (_totalSize > _bytesStored)) {
    createNextPacket();
    size_t bytesAdded = _queue.back().addBinaryData(data, data_left, _payload_pos);
    data_left    -= bytesAdded;
    data         += bytesAdded;
    _bytesStored += bytesAdded;
  }
  return length;
}

size_t ESPEasy_now_splitter::addString(const String& string)
{
  size_t length = string.length() + 1; // Store the extra null-termination

  return addBinaryData(reinterpret_cast<const uint8_t *>(string.c_str()), length);
}

void ESPEasy_now_splitter::createNextPacket()
{
  size_t current_PayloadSize = ESPEasy_Now_packet::getMaxPayloadSize();

  if (_queue.size() > 0) {
    current_PayloadSize = _queue.back().getPayloadSize();
  }

  if (current_PayloadSize > _payload_pos) {
    // No need to create a new file yet

    /*
       String log;
       log = F("createNextPacket ");
       log += data_left;
       addLog(LOG_LEVEL_INFO, log);
     */
    return;
  }

  // Determine size of next packet
  size_t message_bytes_left = _totalSize - _bytesStored;
  _queue.emplace_back(_header, message_bytes_left);
  _payload_pos = 0;

  // Set the packet number for the next packet.
  // Total packet count will be set right before sending them.
  _header.packet_nr++;
}

bool ESPEasy_now_splitter::sendToBroadcast()
{
  uint8_t mac[6];

  for (int i = 0; i < 6; ++i) {
    mac[i] = 0xFF;
  }
  return send(mac);
}

bool ESPEasy_now_splitter::send(uint8_t mac[6])
{
  prepareForSend(mac);

  const size_t nr_packets = _queue.size();

  for (uint8_t i = 0; i < nr_packets; ++i) {
    if (!send(_queue[i])) { return false; }
  }
  return true;
}

WifiEspNowSendStatus ESPEasy_now_splitter::send(uint8_t mac[6], size_t timeout)
{
  START_TIMER;
  prepareForSend(mac);

  WifiEspNowSendStatus sendStatus = WifiEspNowSendStatus::NONE;

  const size_t nr_packets = _queue.size();

  for (uint8_t i = 0; i < nr_packets; ++i) {
    send(_queue[i]);
    sendStatus = waitForSendStatus(timeout);

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;

      switch (sendStatus) {
        case WifiEspNowSendStatus::NONE:
        {
          log = F("ESPEasy Now: TIMEOUT to: ");
          break;
        }
        case WifiEspNowSendStatus::FAIL:
        {
          log = F("ESPEasy Now: Sent FAILED to: ");
          break;
        }
        case WifiEspNowSendStatus::OK:
        {
          log = F("ESPEasy Now: Sent to: ");
          break;
        }
      }
      log += _queue[i].getLogString();
      addLog(LOG_LEVEL_INFO, log);
    }


    switch (sendStatus) {
      case WifiEspNowSendStatus::NONE:
      case WifiEspNowSendStatus::FAIL:
      {
        STOP_TIMER(ESPEASY_NOW_SEND_MSG_FAIL);
        return sendStatus;
      }
      case WifiEspNowSendStatus::OK:
      {
        break;
      }
    }
  }
  STOP_TIMER(ESPEASY_NOW_SEND_MSG_SUC);
  return sendStatus;
}

bool ESPEasy_now_splitter::send(const ESPEasy_Now_packet& packet)
{
  START_TIMER;
  bool has_peer = WifiEspNow.hasPeer(packet._mac);

  if (!has_peer) {
    WifiEspNow.addPeer(packet._mac);
  }
  bool res = WifiEspNow.send(packet._mac, packet[0], packet.getSize());

  if (!has_peer) {
    WifiEspNow.removePeer(packet._mac);
  }
  STOP_TIMER(ESPEASY_NOW_SEND_PCKT);

  delay(0);
  return res;
}

WifiEspNowSendStatus ESPEasy_now_splitter::waitForSendStatus(size_t timeout) const
{
  WifiEspNowSendStatus sendStatus = WifiEspNowSendStatus::NONE;

  while (!timeOutReached(timeout) && sendStatus == WifiEspNowSendStatus::NONE) {
    sendStatus = WifiEspNow.getSendStatus();
    delay(1);
  }
  return sendStatus;
}

void ESPEasy_now_splitter::prepareForSend(uint8_t mac[6])
{
  size_t nr_packets = _queue.size();

  for (uint8_t i = 0; i < nr_packets; ++i) {
    ESPEasy_now_hdr header = _queue[i].getHeader();
    header.nr_packets = nr_packets;
    _queue[i].setHeader(header);
    _queue[i].setMac(mac);
  }
}

#endif // ifdef USES_ESPEASY_NOW
