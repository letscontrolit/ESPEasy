#pragma once

// Packet encoding / decoding of the Grow fingerprint sensor protocol (R503, R307, R502, ZFM-20, AS608, ...).
// No ESPEasy or Arduino dependencies, so it can be unit tested on the host (test/test_protocol.cpp).
//
// Packet: EF 01 | address (4) | PID (1) | length (2) | payload (length - 2) | checksum (2)
//   length   = payload bytes + 2 (the checksum)
//   checksum = 16 bit sum of PID, the two length bytes and the payload
// A command payload is <instruction code> <parameters...>, an acknowledge payload is <confirmation code> <data...>.

#include <stddef.h>
#include <stdint.h>

namespace P190_proto {

constexpr uint8_t  HEADER_HI       = 0xEF;
constexpr uint8_t  HEADER_LO       = 0x01;
constexpr uint32_t DEFAULT_ADDRESS = 0xFFFFFFFF;

constexpr uint8_t PID_COMMAND = 0x01;
constexpr uint8_t PID_ACK     = 0x07;

// Instruction codes
constexpr uint8_t CMD_GEN_IMG       = 0x01; // Capture a finger image
constexpr uint8_t CMD_IMG_2_TZ      = 0x02; // <buffer>: image -> character file in buffer 1 or 2
constexpr uint8_t CMD_SEARCH        = 0x04; // <buffer> <start (2)> <count (2)> -> <page (2)> <score (2)>
constexpr uint8_t CMD_REG_MODEL     = 0x05; // Merge buffers 1 and 2 into a template
constexpr uint8_t CMD_STORE         = 0x06; // <buffer> <page (2)>
constexpr uint8_t CMD_DELETE        = 0x0C; // <page (2)> <count (2)>
constexpr uint8_t CMD_EMPTY         = 0x0D; // Delete all templates
constexpr uint8_t CMD_READ_SYS_PARA = 0x0F; // -> 16 bytes, see SysParams
constexpr uint8_t CMD_VFY_PWD       = 0x13; // <password (4)>
constexpr uint8_t CMD_TEMPLATE_NUM  = 0x1D; // -> <count (2)>
constexpr uint8_t CMD_READ_INDEX    = 0x1F; // <page> -> 32 bytes, bit n = template page * 256 + n is used
constexpr uint8_t CMD_AURA_LED      = 0x35; // <mode> <speed> <color> <times>

// Confirmation codes (first payload byte of an acknowledge)
constexpr uint8_t CODE_OK              = 0x00;
constexpr uint8_t CODE_NO_FINGER       = 0x02;
constexpr uint8_t CODE_NOT_FOUND       = 0x09;
constexpr uint8_t CODE_ENROLL_MISMATCH = 0x0A;
constexpr uint8_t CODE_WRONG_PASSWORD  = 0x13;

// Aura LED ring (R503)
constexpr uint8_t AURA_BREATHING = 1;
constexpr uint8_t AURA_FLASHING  = 2;
constexpr uint8_t AURA_ON        = 3;
constexpr uint8_t AURA_OFF       = 4;
constexpr uint8_t AURA_FADE_IN   = 5;
constexpr uint8_t AURA_FADE_OUT  = 6;

constexpr uint8_t COLOR_RED    = 1;
constexpr uint8_t COLOR_BLUE   = 2;
constexpr uint8_t COLOR_PURPLE = 3;
constexpr uint8_t COLOR_GREEN  = 4; // Green..white: only on the newer full RGB R503 modules
constexpr uint8_t COLOR_YELLOW = 5;
constexpr uint8_t COLOR_CYAN   = 6;
constexpr uint8_t COLOR_WHITE  = 7;

constexpr size_t MAX_PARAMS   = 8;
constexpr size_t MAX_PAYLOAD  = 40;               // Acknowledges are at most 33 bytes (ReadIndexTable)
constexpr size_t MAX_CMD_SIZE = 12 + MAX_PARAMS;  // header 2 + address 4 + PID 1 + length 2 + cmd 1 + checksum 2

// Builds a command packet in buf (at least 12 + nparams bytes). Returns the packet length, 0 if nparams is too large.
inline size_t buildCommand(uint8_t *buf, uint32_t address, uint8_t cmd, const uint8_t *params, size_t nparams) {
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

  for (size_t p = 0; p < nparams; ++p) { buf[i++] = params[p]; }

  uint16_t sum = 0;

  for (size_t p = 6; p < i; ++p) { sum += buf[p]; }
  buf[i++] = static_cast<uint8_t>(sum >> 8);
  buf[i++] = static_cast<uint8_t>(sum);
  return i;
}

struct Packet {
  uint32_t address = 0;
  uint8_t  pid     = 0;
  uint8_t  size    = 0; // payload bytes, checksum excluded
  uint8_t  payload[MAX_PAYLOAD]{};

  uint8_t  code() const               { return size > 0 ? payload[0] : 0xFF; }
  uint16_t word(size_t offset) const  { // big endian 16 bit value at payload[offset]
    return offset + 1 < size ? static_cast<uint16_t>((payload[offset] << 8) | payload[offset + 1]) : 0;
  }
};

// Byte by byte packet decoder: resyncs on the header, drops packets with a bad length or checksum.
class Parser {
public:

  // Returns true when c completes a valid packet, available with packet() until the next feed().
  bool feed(uint8_t c) {
    switch (_pos) {
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

  void          reset()        { _pos = 0; }
  const Packet& packet() const { return _pkt; }
  uint32_t      errors() const { return _errors; }

private:

  Packet   _pkt;
  uint8_t  _pos      = 0;
  uint8_t  _received = 0;
  uint16_t _length   = 0;
  uint16_t _sum      = 0;
  uint16_t _checksum = 0;
  uint32_t _errors   = 0;
};

// Template index table: one bit per template, bit 0 of byte 0 = template 0 (as sent by ReadIndexTable)
constexpr uint16_t INDEX_PAGE_TEMPLATES = 256;
constexpr size_t   INDEX_PAGE_BYTES     = INDEX_PAGE_TEMPLATES / 8;

inline bool indexUsed(const uint8_t *table, uint16_t id) {
  return (table[id >> 3] >> (id & 7)) & 1;
}

// First free template below capacity, -1 if all are used
inline int indexFirstFree(const uint8_t *table, uint16_t capacity) {
  for (uint16_t id = 0; id < capacity; ++id) {
    if (!indexUsed(table, id)) { return id; }
  }
  return -1;
}

// Case insensitive match of a name in a list, or a number in 1..count. Returns 1..count, 0 if not found.
inline uint8_t lookup(const char *arg, const char *const *names, uint8_t count) {
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

    while (*a && *n && ((*a | 0x20) == *n)) { ++a; ++n; } // names are lower case

    if ((*a == '\0') && (*n == '\0')) { return i + 1; }
  }
  return 0;
}

inline uint8_t ledMode(const char *arg) {
  static const char *const names[] = { "breathing", "flash", "on", "off", "fadein", "fadeout" };

  return lookup(arg, names, 6);
}

inline uint8_t ledColor(const char *arg) {
  static const char *const names[] = { "red", "blue", "purple", "green", "yellow", "cyan", "white" };

  return lookup(arg, names, 7);
}

} // namespace P190_proto
