#include "../PluginStructs/P190_protocol.h"

// Built with the plugin, or on its own (P190_HOST_TEST) for the host unit tests of the protocol
#ifndef P190_HOST_TEST
# include "../../_Plugin_Helper.h"
#endif // ifndef P190_HOST_TEST

#if defined(USES_P190) || defined(P190_HOST_TEST)

namespace P190_proto {

size_t buildCommand(uint8_t *buf, uint32_t address, uint8_t cmd, const uint8_t *params, size_t nparams) {
  if (nparams > MAX_PARAMS) { return 0; }
  const uint16_t length = static_cast<uint16_t>(nparams + 3); // cmd + params + checksum
  size_t i              = 0;

  buf[i++] = HEADER_HI;
  buf[i++] = HEADER_LO;
  buf[i++] = static_cast<uint8_t>(address >> 24);
  buf[i++] = static_cast<uint8_t>(address >> 16);
  buf[i++] = static_cast<uint8_t>(address >> 8);
  buf[i++] = static_cast<uint8_t>(address);
  buf[i++] = PID_COMMAND;
  buf[i++] = static_cast<uint8_t>(length >> 8);
  buf[i++] = static_cast<uint8_t>(length);
  buf[i++] = cmd;

  for (size_t p = 0; p < nparams; ++p) {
    buf[i++] = params[p];
  }

  uint16_t sum = 0;

  for (size_t p = 6; p < i; ++p) {
    sum += buf[p];
  }
  buf[i++] = static_cast<uint8_t>(sum >> 8);
  buf[i++] = static_cast<uint8_t>(sum);
  return i;
}

bool Parser::feed(uint8_t c) {
  switch (_pos)
  {
    case 0:

      if (c == HEADER_HI) { _pos = 1; }
      return false;
    case 1:

      if (c == HEADER_LO) { _pos = 2; }
      else if (c != HEADER_HI) { _pos = 0; }
      return false;
    case 2: case 3: case 4: case 5:
      _pkt.address = (_pos == 2 ? 0 : _pkt.address << 8) | c;
      ++_pos;
      return false;
    case 6:
      _pkt.pid = c;
      _sum     = c;
      ++_pos;
      return false;
    case 7:
      _length = static_cast<uint16_t>(c << 8);
      _sum   += c;
      ++_pos;
      return false;
    case 8:
      _length |= c;
      _sum    += c;

      if ((_length < 2) || (_length > MAX_PAYLOAD + 2)) {
        ++_errors;
        _pos = 0;
        return false;
      }
      _pkt.size = static_cast<uint8_t>(_length - 2);
      _received = 0;
      _pos      = _pkt.size > 0 ? 9 : 10;
      return false;
    case 9:
      _pkt.payload[_received++] = c;
      _sum                     += c;

      if (_received == _pkt.size) { _pos = 10; }
      return false;
    case 10:
      _checksum = static_cast<uint16_t>(c << 8);
      _pos      = 11;
      return false;
    default: // 11
      _checksum |= c;
      _pos       = 0;

      if (_checksum != _sum) {
        ++_errors;
        return false;
      }
      return true;
  }
}

bool indexUsed(const uint8_t *table, uint16_t id) { return (table[id >> 3] >> (id & 7)) & 1; }

int  indexFirstFree(const uint8_t *table, uint16_t capacity) {
  for (uint16_t id = 0; id < capacity; ++id) {
    if (!indexUsed(table, id)) { return id; }
  }
  return -1;
}

uint8_t lookup(const char *arg, const char *const *names, uint8_t count) {
  if ((arg == nullptr) || (*arg == '\0')) { return 0; }

  if ((*arg >= '0') && (*arg <= '9')) {
    unsigned value = 0;

    for (const char *p = arg; *p; ++p) {
      if ((*p < '0') || (*p > '9') || (value > 255)) { return 0; }
      value = value * 10 + (*p - '0');
    }
    return (value >= 1 && value <= count) ? static_cast<uint8_t>(value) : 0;
  }

  for (uint8_t i = 0; i < count; ++i) {
    const char *a = arg;
    const char *n = names[i];

    // names are lower case
    while (*a && *n && ((*a | 0x20) == *n)) {
      ++a;
      ++n;
    }

    if ((*a == '\0') && (*n == '\0')) { return i + 1; }
  }
  return 0;
}

uint8_t ledMode(const char *arg) {
  static const char *const names[] = { "breathing", "flash", "on", "off", "fadein", "fadeout" };

  return lookup(arg, names, 6);
}

uint8_t ledColor(const char *arg) {
  static const char *const names[] = { "red", "blue", "purple", "green", "yellow", "cyan", "white" };

  return lookup(arg, names, 7);
}

} // namespace P190_proto

#endif // if defined(USES_P190) || defined(P190_HOST_TEST)
