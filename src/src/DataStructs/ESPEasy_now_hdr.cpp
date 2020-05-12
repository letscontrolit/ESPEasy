#include "ESPEasy_now_hdr.h"

#ifdef USES_ESPEASY_NOW

ESPEasy_now_hdr::ESPEasy_now_hdr() {}

ESPEasy_now_hdr::ESPEasy_now_hdr(ESPEasy_now_hdr::message_t messageType)
  : message_type(messageType)  {}

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
  res ^= cur_message_nr;
  res ^= last_message_nr;
  res ^= notUsed1;
  res ^= notUsed2;
  return res;
}

#endif // ifdef USES_ESPEASY_NOW
