#include "ESPEasy_now_hdr.h"

#ifdef USES_ESPEASY_NOW

ESPEasy_now_hdr::ESPEasy_now_hdr() {}

ESPEasy_now_hdr::ESPEasy_now_hdr(ESPEasy_now_hdr::message_t messageType)
  : message_type(messageType)  {}

ESPEasy_now_hdr& ESPEasy_now_hdr::operator=(const ESPEasy_now_hdr& other)
{
  if (&other == this) {
    return *this;
  }
  header_version = other.header_version;
  message_type   = other.message_type;
  packet_nr      = other.packet_nr;
  nr_packets     = other.nr_packets;
  message_count  = other.message_count;
  payload_size   = other.payload_size;
  checksum       = other.checksum;
  return *this;
}

#endif // ifdef USES_ESPEASY_NOW
