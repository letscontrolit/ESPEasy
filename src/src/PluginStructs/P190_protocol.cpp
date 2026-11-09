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

// Each byte fills the current field of the packet, then moves on to the next one.
// Any error goes back to waiting for a header.
bool Parser::feed(uint8_t c) {
  switch (_field)
  {
    case Field::HeaderHi:

      if (c == HEADER_HI) { _field = Field::HeaderLo; }
      break;
    case Field::HeaderLo:

      if (c == HEADER_LO) {
        _field       = Field::Address;
        _pkt.address = 0;
        _received    = 0;
      } else if (c != HEADER_HI) { // EF EF 01: the second EF can still start the header
        _field = Field::HeaderHi;
      }
      break;
    case Field::Address: // big endian
      _pkt.address = (_pkt.address << 8) | c;

      if (++_received == 4) { _field = Field::Pid; }
      break;
    case Field::Pid: // the checksum covers PID, length and payload
      _pkt.pid = c;
      _sum     = c;
      _field   = Field::LengthHi;
      break;
    case Field::LengthHi:
      _length = static_cast<uint16_t>(c << 8);
      _sum   += c;
      _field  = Field::LengthLo;
      break;
    case Field::LengthLo: // payload + 2 checksum bytes
      _length |= c;
      _sum    += c;

      if ((_length < 2) || (_length > MAX_PAYLOAD + 2)) {
        ++_errors;
        _field = Field::HeaderHi;
        break;
      }
      _pkt.size = static_cast<uint8_t>(_length - 2);
      _received = 0;
      _field    = _pkt.size > 0 ? Field::Payload : Field::ChecksumHi;
      break;
    case Field::Payload:
      _pkt.payload[_received++] = c;
      _sum                     += c;

      if (_received == _pkt.size) { _field = Field::ChecksumHi; }
      break;
    case Field::ChecksumHi:
      _checksum = static_cast<uint16_t>(c << 8);
      _field    = Field::ChecksumLo;
      break;
    case Field::ChecksumLo: // packet complete
      _checksum |= c;
      _field     = Field::HeaderHi;

      if (_checksum == _sum) { return true; }
      ++_errors;
      break;
  }
  return false;
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
