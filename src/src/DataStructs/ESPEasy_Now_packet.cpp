#include "ESPEasy_Now_packet.h"

#ifdef USES_ESPEASY_NOW


# define ESPEASY_NOW_MAX_PACKET_SIZE   250

ESPEasy_Now_packet::ESPEasy_Now_packet(const ESPEasy_now_hdr &header, size_t payloadSize)
{
  setSize(payloadSize + sizeof(ESPEasy_now_hdr));
  setHeader(header);
}

ESPEasy_Now_packet::ESPEasy_Now_packet(const uint8_t mac[6], const uint8_t *buf, size_t packetSize)
{
  setSize(packetSize);
  memcpy(_mac,     mac,          6);
  memcpy(&_buf[0], buf, packetSize);
}

void ESPEasy_Now_packet::setSize(size_t packetSize)
{
  if (packetSize > ESPEASY_NOW_MAX_PACKET_SIZE) {
    _buf.resize(ESPEASY_NOW_MAX_PACKET_SIZE);
  }
  else {
    _buf.resize(packetSize);
  }
}

size_t ESPEasy_Now_packet::getSize() const
{
  return _buf.size();
}

size_t ESPEasy_Now_packet::getPayloadSize() const
{
  size_t size = getSize();
  if (size < sizeof(ESPEasy_now_hdr)) {
    // should not happen
    return 0;
  }
  return size - sizeof(ESPEasy_now_hdr);
}

ESPEasy_now_hdr ESPEasy_Now_packet::getHeader() const
{
  ESPEasy_now_hdr header;
  if (getSize() >= sizeof(ESPEasy_now_hdr)) {
    memcpy(&header, &_buf[0], sizeof(ESPEasy_now_hdr));
  }
  if (!header.checksumValid()) {
    header.message_type = ESPEasy_now_hdr::message_t::ChecksumError;
  }
  return header;
}

void ESPEasy_Now_packet::setHeader(ESPEasy_now_hdr header)
{
  header.setChecksum();
  memcpy(&_buf[0], &header, sizeof(ESPEasy_now_hdr));
}

size_t ESPEasy_Now_packet::addString(const String& string, size_t payload_pos)
{
  const size_t payload_size = getPayloadSize();

  if (payload_pos > payload_size) {
    return 0;
  }
  size_t bytesToWrite = string.length() + 1; // include null-termination
  const size_t payload_free = payload_size - payload_pos;

  if (bytesToWrite > payload_free) {
    bytesToWrite = payload_free;
  }

  // Copy the string including null-termination.
  // If the null-termination does not fit, no other string can be added anyway.
  size_t buf_pos      = payload_pos + sizeof(ESPEasy_now_hdr);
  memcpy(&_buf[buf_pos], reinterpret_cast<const uint8_t *>(string.c_str()), bytesToWrite);
  return bytesToWrite;
}

String ESPEasy_Now_packet::getString(size_t payload_pos) const
{
  String res;
  const size_t size = getSize();
  size_t buf_pos = payload_pos + sizeof(ESPEasy_now_hdr);

  while (buf_pos < size && _buf[buf_pos] == 0) {
    ++buf_pos;
  }

  if (buf_pos >= size) { return res; }

  const size_t maxlen = size - buf_pos;
  size_t strlength    = strnlen(reinterpret_cast<const char *>(&_buf[buf_pos]), maxlen);
  res.reserve(strlength);

  for (size_t i = 0; i < strlength; ++i) {
    res += static_cast<char>(_buf[buf_pos + i]);
  }
  return res;
}

#endif // ifdef USES_ESPEASY_NOW
