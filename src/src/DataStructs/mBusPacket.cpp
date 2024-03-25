#include "../DataStructs/mBusPacket.h"

#include "../Helpers/CRC_functions.h"
#include "../Helpers/StringConverter.h"

#define FRAME_FORMAT_A_FIRST_BLOCK_LENGTH 10
#define FRAME_FORMAT_A_OTHER_BLOCK_LENGTH 16


mBusPacket_header_t::mBusPacket_header_t()
{
  _manufacturer = mBus_packet_wildcard_manufacturer;
  _meterType    = mBus_packet_wildcard_metertype;
  _serialNr     = mBus_packet_wildcard_serial;
  _length       = 0u;
}

String mBusPacket_header_t::decodeManufacturerID(int id)
{
  String res;
  int    shift = 15;

  for (int i = 0; i < 3; ++i) {
    shift -= 5;
    res   += static_cast<char>(((id >> shift) & 0x1f) + 64);
  }
  return res;
}

int mBusPacket_header_t::encodeManufacturerID(const String& id_str)
{
  int res     = 0;
  int nrChars = id_str.length();

  if (nrChars > 3) { nrChars = 3; }

  int i = 0;

  while (i < nrChars) {
    res <<= 5;
    const int c = static_cast<int>(toUpperCase(id_str[i])) - 64;

    if (c >= 0) {
      res += c & 0x1f;
    }
    ++i;
  }
  return res;
}

String mBusPacket_header_t::getManufacturerId() const
{
  return decodeManufacturerID(_manufacturer);
}

String mBusPacket_header_t::toString() const
{
  String res = decodeManufacturerID(_manufacturer);

  res += '.';
  res += formatToHex_no_prefix(_meterType, 2);
  res += '.';
  res += formatToHex_no_prefix(_serialNr, 8);
  return res;
}

uint64_t mBusPacket_header_t::encode_toUInt64() const
{
  if (!isValid()) { return 0ull; }
  mBusPacket_header_t tmp(*this);

  tmp._length = 0;
  return tmp._encodedValue;
}

void mBusPacket_header_t::decode_fromUint64(uint64_t encodedValue)
{
  _encodedValue = encodedValue;
  _length       = 1; // To pass isValid() check
}

bool mBusPacket_header_t::isValid() const
{
  return
    _manufacturer != mBus_packet_wildcard_manufacturer &&
    _meterType != mBus_packet_wildcard_metertype &&
    _serialNr != mBus_packet_wildcard_serial &&
    _length > 0;
}

void mBusPacket_header_t::clear()
{
  _manufacturer = mBus_packet_wildcard_manufacturer;
  _meterType    = mBus_packet_wildcard_metertype;
  _serialNr     = mBus_packet_wildcard_serial;
  _length       = 0;
}

bool mBusPacket_header_t::matchSerial(uint32_t serialNr) const
{
  return isValid() && (_serialNr == serialNr);
}

const mBusPacket_header_t * mBusPacket_t::getDeviceHeader() const
{
  // FIXME TD-er: Which deviceID is the device and which the wrapper?
  if (_deviceId1.isValid()) { return &_deviceId1; }

  if (_deviceId2.isValid()) { return &_deviceId2; }

  return nullptr;
}

uint32_t mBusPacket_t::getDeviceSerial() const
{
  const mBusPacket_header_t *header = getDeviceHeader();

  if (header == nullptr) { return 0u; }
  return header->_serialNr;
}

uint32_t mBusPacket_t::deviceID_to_map_key() const
{
  return deviceID_to_map_key(_deviceId1._encodedValue, _deviceId2._encodedValue);
}

uint32_t mBusPacket_t::deviceID_to_map_key_no_length() const {
  return deviceID_to_map_key(_deviceId1.encode_toUInt64(), _deviceId2.encode_toUInt64());
}

uint32_t mBusPacket_t::deviceID_to_map_key(uint64_t id1, uint64_t id2)
{
  uint32_t res = 0;

  if (id1 != 0ull) {
    res ^= calc_CRC32((const uint8_t *)(&id1), sizeof(uint64_t));
  }

  if (id2 != 0ull) {
    // There is a forwarding device.
    // To prevent issues when the forwarding device is the same as the forwarded device, alter the already existing checksum.
    res ^= calc_CRC32((const uint8_t *)(&res), sizeof(res));
    res ^= calc_CRC32((const uint8_t *)(&id2), sizeof(uint64_t));
  }

  return res;
}

bool mBusPacket_t::parse(const String& payload)
{
  if (payload[0] != 'b') { return false; }

  _checksum = 0;
  mBusPacket_data payloadWithoutChecksums;

  if (payload[1] == 'Y') {
    // Start with "bY"
    payloadWithoutChecksums = removeChecksumsFrameB(payload, _checksum);
  } else {
    payloadWithoutChecksums = removeChecksumsFrameA(payload, _checksum);
  }

  if (payloadWithoutChecksums.size() < 10) { return false; }

  int pos_semicolon = payload.indexOf(';');

  if (pos_semicolon == -1) { pos_semicolon = payload.length(); }

  _lqi_rssi = hexToUL(payload, pos_semicolon - 4, 4);
  return parseHeaders(payloadWithoutChecksums);
}

int16_t mBusPacket_t::decode_LQI_RSSI(uint16_t lqi_rssi, uint8_t& LQI)
{
  LQI = (lqi_rssi >> 8) & 0x7f; // Bit 7 = CRC OK Bit

  int rssi = lqi_rssi & 0xFF;

  if (rssi >= 128) {
    rssi -= 256; // 2-complement
  }
  return (rssi / 2) - 74;
}

bool mBusPacket_t::matchSerial(uint32_t serialNr) const
{
  return _deviceId1.matchSerial(serialNr) || _deviceId2.matchSerial(serialNr);
}

bool mBusPacket_t::parseHeaders(const mBusPacket_data& payloadWithoutChecksums)
{
  const int payloadSize = payloadWithoutChecksums.size();

  _deviceId1.clear();
  _deviceId2.clear();

  if (payloadSize < 10) { return false; }
  int offset = 0;

  // 1st block is a static DataLinkLayer of 10 bytes
  {
    _deviceId1._manufacturer = makeWord(payloadWithoutChecksums[offset + 3], payloadWithoutChecksums[offset + 2]);

    // Type (offset + 9; convert to hex)
    _deviceId1._meterType = payloadWithoutChecksums[offset + 9];

    // Serial (offset + 4; 4 Bytes; least significant first; converted to hex)
    _deviceId1._serialNr = 0;

    for (int i = 0; i < 4; ++i) {
      const uint32_t val = payloadWithoutChecksums[offset + 4 + i];
      _deviceId1._serialNr += val << (i * 8);
    }
    offset            += 10;
    _deviceId1._length = payloadWithoutChecksums[0];
  }

  // next blocks can be anything. we skip known blocks of no interest, parse known blocks if interest and stop on onknown blocks
  while (offset < payloadSize) {
    switch (static_cast<int>(payloadWithoutChecksums[offset])) {
      case 0x8C:                                            // ELL short
        offset            += 3;                             // fixed length
        _deviceId1._length = payloadSize - offset;
        break;
      case 0x90:                                            // AFL
        offset++;
        offset += (payloadWithoutChecksums[offset] & 0xff); // dynamic length with length in 1st byte
        offset++;                                           // length byte
        _deviceId1._length = payloadSize - offset;
        break;
      case 0x72:                                            // TPL_RESPONSE_MBUS_LONG_HEADER
        _deviceId2 = _deviceId1;

        // note that serial/manufacturer are swapped !!

        _deviceId1._manufacturer = makeWord(payloadWithoutChecksums[offset + 6], payloadWithoutChecksums[offset + 5]);

        // Type (offset + 9; convert to hex)
        _deviceId1._meterType = payloadWithoutChecksums[offset + 8];

        // Serial (offset + 4; 4 Bytes; least significant first; converted to hex)
        _deviceId1._serialNr = 0;


        for (int i = 0; i < 4; ++i) {
          const uint32_t val = payloadWithoutChecksums[offset + 1 + i];
          _deviceId1._serialNr += val << (i * 8);
        }

        // We're done
        offset = payloadSize;
        break;
      default:
        // We're done
        //        addLog(LOG_LEVEL_ERROR, concat(F("CUL : offset "), offset) + F(" Data: ") + formatToHex(payloadWithoutChecksums[offset]));
        offset = payloadSize;
        break;
    }
  }

  if (_deviceId1.isValid() && _deviceId2.isValid() && _deviceId1.toString().startsWith(F("ITW.30."))) {
    // ITW does not follow the spec and puts the redio converter behind the actual meter. Need to swap both
    std::swap(_deviceId1, _deviceId2);
  }

  return _deviceId1.isValid();
}

String mBusPacket_t::toString() const
{
  static size_t expectedSize = 96;
  String res;

  if (res.reserve(expectedSize)) {
    if (_deviceId1.isValid()) {
      res += F(" deviceId1: ");
      res += _deviceId1.toString();
      res += '(';
      res += static_cast<uint8_t>(_deviceId1._length);
      res += ')';
    }

    if (_deviceId2.isValid()) {
      res += F(" deviceId2: ");
      res += _deviceId2.toString();
      res += '(';
      res += static_cast<uint8_t>(_deviceId2._length);
      res += ')';
    }
    res += F(" chksum: ");
    res += formatToHex(_checksum, 8);

    uint8_t LQI        = 0;
    const int16_t rssi = decode_LQI_RSSI(_lqi_rssi, LQI);
    res += F(" LQI: ");
    res += LQI;
    res += F(" RSSI: ");
    res += rssi;
  }

  if (res.length() > expectedSize) { expectedSize = res.length(); }

  return res;
}

uint8_t mBusPacket_t::hexToByte(const String& str, size_t index)
{
  // Need to have at least 2 HEX nibbles
  if ((index + 1) >= str.length()) { return 0; }
  return hexToUL(str, index, 2);
}

/**
 * Format:
 * [10 bytes message] + [2 bytes CRC]
 * [16 bytes message] + [2 bytes CRC]
 * [16 bytes message] + [2 bytes CRC]
 * ...
 * (last block can be < 16 bytes)
 */
mBusPacket_data mBusPacket_t::removeChecksumsFrameA(const String& payload, uint32_t& checksum)
{
  mBusPacket_data result;
  const int payloadLength = payload.length();

  if (payloadLength < 4) { return result; }

  int sourceIndex = 1; // Starts with "b"
  int targetIndex = 0;

  // 1st byte contains length of data (excuding 1st byte and excluding CRC)
  const int expectedMessageSize = hexToByte(payload, sourceIndex) + 1;

  if (payloadLength < (2 * expectedMessageSize)) {
    // Not an exact check, but close enough to fail early on packets which are seriously too short.
    return result;
  }

  result.reserve(expectedMessageSize);

  while (targetIndex < expectedMessageSize) {
    // end index is start index + block size + 2 byte checksums
    int blockSize = (sourceIndex == 1) ? FRAME_FORMAT_A_FIRST_BLOCK_LENGTH : FRAME_FORMAT_A_OTHER_BLOCK_LENGTH;

    if ((targetIndex + blockSize) > expectedMessageSize) { // last block
      blockSize = expectedMessageSize - targetIndex;
    }

    // FIXME: handle truncated source messages
    for (int i = 0; i < blockSize; ++i) {
      result.push_back(hexToByte(payload, sourceIndex));
      sourceIndex += 2; // 2 hex chars
    }

    // [2 bytes CRC]
    checksum   <<= 8;
    checksum    ^= hexToUL(payload, sourceIndex, 4);
    sourceIndex += 4; // Skip 2 bytes CRC => 4 hex chars
    targetIndex += blockSize;
  }
  return result;
}

/**
 * Format:
 * [126 bytes message] + [2 bytes CRC]
 * [125 bytes message] + [2 bytes CRC]
 * (if message length <=126 bytes, only the 1st block exists)
 * (last block can be < 125 bytes)
 */
mBusPacket_data mBusPacket_t::removeChecksumsFrameB(const String& payload, uint32_t& checksum)
{
  mBusPacket_data result;
  const int payloadLength = payload.length();

  if (payloadLength < 4) { return result; }

  int sourceIndex = 2; // Starts with "bY"

  // 1st byte contains length of data (excuding 1st byte BUT INCLUDING CRC)
  int expectedMessageSize = hexToByte(payload, sourceIndex) + 1;

  if (payloadLength < (2 * expectedMessageSize)) {
    return result;
  }

  expectedMessageSize -= 2;   // CRC of 1st block

  if (expectedMessageSize > 128) {
    expectedMessageSize -= 2; // CRC of 2nd block
  }

  result.reserve(expectedMessageSize);

  // FIXME: handle truncated source messages

  const int block1Size = expectedMessageSize < 126 ? expectedMessageSize : 126;

  for (int i = 0; i < block1Size; ++i) {
    result.push_back(hexToByte(payload, sourceIndex));
    sourceIndex += 2; // 2 hex chars
  }

  // [2 bytes CRC]
  checksum   <<= 8;
  checksum    ^= hexToUL(payload, sourceIndex, 4);
  sourceIndex += 4; // Skip 2 bytes CRC => 4 hex chars

  if (expectedMessageSize > 126) {
    int block2Size = expectedMessageSize - 127;

    if (block2Size > 124) { block2Size = 124; }

    for (int i = 0; i < block2Size; ++i) {
      result.push_back(hexToByte(payload, sourceIndex));
      sourceIndex += 2; // 2 hex chars
    }

    // [2 bytes CRC]
    checksum <<= 8;
    checksum  ^= hexToUL(payload, sourceIndex, 4);
  }

  // remove the checksums and the 1st byte from the actual message length, so that the meaning of this byte is the same as in Frame A
  result[0] = static_cast<uint8_t>((expectedMessageSize - 1) & 0xff);

  return result;
}
