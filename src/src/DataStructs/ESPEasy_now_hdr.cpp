#include "ESPEasy_now_hdr.h"

#ifdef USES_ESPEASY_NOW

ESPEasy_now_hdr::ESPEasy_now_hdr() {}

ESPEasy_now_hdr::ESPEasy_now_hdr(ESPEasy_now_hdr::message_t messageType)
  : message_type(messageType)  {}

ESPEasy_now_hdr& ESPEasy_now_hdr::operator=(const ESPEasy_now_hdr &other)
{
  if(&other == this)
    return *this;
  header_version = other.header_version;
  message_type = other.message_type;
  packet_nr = other.packet_nr;
  nr_packets = other.nr_packets;
  message_count = other.message_count;
  notUsed1 = other.notUsed1;
  checksum = other.checksum;
  return *this;
}

void ESPEasy_now_hdr::setChecksum()
{
  checksum = computeChecksum();
}

bool ESPEasy_now_hdr::checksumValid() const
{
  return computeChecksum() == checksum;
}

uint8_t ESPEasy_now_hdr::computeChecksum() const
{
  // TODO TD-er: Maybe better to have this as a for loop over *this
  uint8_t res = header_version;
  res ^= static_cast<uint8_t>(message_type);
  res ^= packet_nr;
  res ^= nr_packets;
  res ^= message_count;
  res ^= notUsed1;
  return res;
}

#endif // ifdef USES_ESPEASY_NOW
