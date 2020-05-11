#include "ESPEasy_Now_incoming.h"

ESPEasy_Now_incoming::ESPEasy_Now_incoming(const uint8_t mac[6], const uint8_t *buf, size_t count)
{
  _buf.resize(count);
  memcpy(_mac,     mac,     6);
  memcpy(&_buf[0], buf, count);
}

size_t ESPEasy_Now_incoming::getSize() const
{
  return _buf.size();
}

String ESPEasy_Now_incoming::getString(size_t pos) const
{
  String res;
  const size_t size = getSize();

  while (pos < size && _buf[pos] == 0) {
    ++pos;
  }

  if (pos >= size) { return res; }

  const size_t maxlen = size - pos;
  size_t strlength    = strnlen(reinterpret_cast<const char *>(&_buf[pos]), maxlen);
  res.reserve(strlength);

  for (size_t i = 0; i < strlength; ++i) {
    res += static_cast<char>(_buf[pos + i]);
  }
  return res;
}
