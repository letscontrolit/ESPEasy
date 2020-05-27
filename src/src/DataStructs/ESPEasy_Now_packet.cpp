#include "ESPEasy_Now_packet.h"

#ifdef USES_ESPEASY_NOW

# include "../../ESPEasy_fdwdecl.h"

# define ESPEASY_NOW_MAX_PACKET_SIZE   200

ESPEasy_Now_packet::ESPEasy_Now_packet(const ESPEasy_now_hdr& header, size_t payloadSize)
{
  setSize(payloadSize + sizeof(ESPEasy_now_hdr));
  setHeader(header);
}

ESPEasy_Now_packet::ESPEasy_Now_packet(const MAC_address& mac, const uint8_t *buf, size_t packetSize)
{
  setSize(packetSize);
  mac.get(_mac);
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

uint16_t ESPEasy_Now_packet::computeChecksum() const
{
  return calc_CRC16(reinterpret_cast<const char *>(begin()), getPayloadSize());
}

bool ESPEasy_Now_packet::checksumValid() const
{
  return getHeader().checksum == computeChecksum();
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

size_t ESPEasy_Now_packet::getMaxPayloadSize()
{
  return ESPEASY_NOW_MAX_PACKET_SIZE - sizeof(ESPEasy_now_hdr);
}

ESPEasy_now_hdr ESPEasy_Now_packet::getHeader() const
{
  ESPEasy_now_hdr header;

  if (getSize() >= sizeof(ESPEasy_now_hdr)) {
    memcpy(&header, &_buf[0], sizeof(ESPEasy_now_hdr));
  }

  return header;
}

void ESPEasy_Now_packet::setHeader(ESPEasy_now_hdr header)
{
  header.checksum = computeChecksum();
  memcpy(&_buf[0], &header, sizeof(ESPEasy_now_hdr));
}

size_t ESPEasy_Now_packet::addString(const String& string, size_t& payload_pos)
{
  size_t length = string.length() + 1; // Try to Store the extra null-termination (may fail)

  return addBinaryData(reinterpret_cast<const uint8_t *>(string.c_str()), length, payload_pos);
}

void ESPEasy_Now_packet::setMac(const MAC_address& mac)
{
  memcpy(_mac, mac.mac, 6);
}

void ESPEasy_Now_packet::setBroadcast()
{
  for (byte i = 0; i < 6; ++i)
  {
    _mac[i] = 0xFF;
  }
}

size_t ESPEasy_Now_packet::addBinaryData(const uint8_t *data, size_t length,
                                         size_t& payload_pos)
{
  size_t bytes_left = getPayloadSize();

  if (payload_pos > bytes_left) { return 0; }
  bytes_left -= payload_pos;

  if (length > bytes_left) {
    length = bytes_left;
  }
  size_t buf_pos =  sizeof(ESPEasy_now_hdr) + payload_pos;
  memcpy(&_buf[buf_pos], data, length);
  payload_pos += length;
  return length;
}

size_t ESPEasy_Now_packet::getBinaryData(uint8_t *data, size_t length,
                                         size_t& payload_pos) const
{
  size_t bytes_left = getPayloadSize();

  if (payload_pos > bytes_left) { return 0; }
  bytes_left -= payload_pos;

  if (length > bytes_left) {
    length = bytes_left;
  }
  size_t buf_pos =  sizeof(ESPEasy_now_hdr) + payload_pos;
  memcpy(data, &_buf[buf_pos], length);
  payload_pos += length;
  return length;
}

String ESPEasy_Now_packet::getString(size_t& payload_pos) const
{
  String res;
  size_t bytes_left = getPayloadSize();

  if (payload_pos > bytes_left) { return res; }
  bytes_left -= payload_pos;
  size_t buf_pos   =  sizeof(ESPEasy_now_hdr) + payload_pos;
  size_t strlength = strnlen(reinterpret_cast<const char *>(&_buf[buf_pos]), bytes_left);
  res.reserve(strlength);
  const size_t max_buf_pos = buf_pos + strlength;

  for (; buf_pos < max_buf_pos; ++buf_pos, ++payload_pos) {
    res += static_cast<char>(_buf[buf_pos]);
  }
  return res;
}

String ESPEasy_Now_packet::getLogString() const
{
  ESPEasy_now_hdr header = getHeader();
  String log;

  log.reserve(40);
  log += MAC_address(_mac).toString();
  log += F(" payload: ");
  log += getPayloadSize();
  log += F(" (");
  log += header.packet_nr + 1;
  log += '/';
  log += header.nr_packets;
  log += ')';
  return log;
}

const uint8_t * ESPEasy_Now_packet::begin() const
{
  return &_buf[sizeof(ESPEasy_now_hdr)];
}

#endif // ifdef USES_ESPEASY_NOW
