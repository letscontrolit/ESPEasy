#include "ESPEasy_now_splitter.h"

#ifdef USES_ESPEASY_NOW

static uint8_t ESPEasy_now_message_count = 1;

ESPEasy_now_splitter::ESPEasy_now_splitter(ESPEasy_now_hdr::message_t message_type, size_t totalSize)
  : _header(message_type), _totalSize(totalSize)
{
  _header.message_count = ++ESPEasy_now_message_count;
}

size_t ESPEasy_now_splitter::addBinaryData(const uint8_t *data, size_t length)
{
  size_t data_left = length;

  while (data_left > 0) {
    createNextPacket(data_left);
    size_t bytesAdded = _queue.back().addBinaryData(data, data_left);
    data_left    -= bytesAdded;
    data         += bytesAdded;
    _payload_pos += bytesAdded;
    _bytesStored += bytesAdded;
  }
  return length;
}

size_t ESPEasy_now_splitter::addString(const String& string)
{
  size_t length = string.length() + 1; // Store the extra null-termination
  return addBinaryData(reinterpret_cast<const uint8_t *>(string.c_str()), length);
}

bool ESPEasy_now_splitter::send(uint8_t mac[6])
{}


void ESPEasy_now_splitter::createNextPacket(size_t data_left)
{
  size_t maxPayloadSize = ESPEasy_Now_packet::getMaxPayloadSize() - 1;
  
  if (maxPayloadSize > _payload_pos) {
    if ((_payload_pos + data_left) == maxPayloadSize) {
      // Special case where we must decrease next packet slightly
      maxPayloadSize -= 2;
    } else {
      // No need to create a new file yet    
      return;
    }
  }

  // Determine size of next packet
  size_t message_bytes_left = _totalSize - _bytesStored;
  size_t packetSize         = message_bytes_left;

  if (packetSize > maxPayloadSize) {
    packetSize = maxPayloadSize;
  }
  if (packetSize == data_left) {
    packetSize -= 2;
  }
  _queue.emplace_back(_header, packetSize);
  _payload_pos = 0;

  // Set the packet number for the next packet.
  // Total packet count will be set right before sending them.
  _header.packet_nr++;
}

void                 ESPEasy_now_splitter::setMac(uint8_t mac[6])
{}

bool                 ESPEasy_now_splitter::send(const ESPEasy_Now_packet& packet)
{}

WifiEspNowSendStatus ESPEasy_now_splitter::send(const ESPEasy_Now_packet& packet,
                                                size_t                    timeout)
{}

WifiEspNowSendStatus ESPEasy_now_splitter::waitForSendStatus(size_t timeout) const
{}


#endif // ifdef USES_ESPEASY_NOW
