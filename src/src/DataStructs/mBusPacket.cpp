#include "../DataStructs/mBusPacket.h"


#include "../Helpers/StringConverter.h"

#define FRAME_FORMAT_A_FIRST_BLOCK_LENGTH 10
#define FRAME_FORMAT_A_OTHER_BLOCK_LENGTH 16

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

  for (int i = 0; i < nrChars; ++i) {
    const char c = id_str[i];

    if ((c >= 64) && (c < 96)) {
      res += static_cast<int>(c) - 64;
    }
    res <<= 5;
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

bool mBusPacket_header_t::isValid() const
{
  return _manufacturer != 0 &&
         _meterType != 0 &&
         _serialNr != 0 &&
         _length > 0;
}

bool mBusPacket_t::parse(const String& payload)
{
  if (payload.startsWith(F("bY"))) {
    return parseHeaders(removeChecksumsFrameB(payload));
  }

  if (payload.startsWith(F("b"))) {
    return parseHeaders(removeChecksumsFrameA(payload));
  }
  return false;
}

bool mBusPacket_t::parseHeaders(const mBusPacket_data& payloadWithoutChecksums)
{
  const size_t payloadSize = payloadWithoutChecksums.size();


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
    offset += 10;
    _deviceId1._length = offset;
  }

  // next blocks can be anything. we skip known blocks of no interest, parse known blocks if interest and stop on onknown blocks
  while (offset < payloadSize) {
    switch (payloadWithoutChecksums[offset]) {
      case 0x8C:                                            // ELL short
        offset += 3;                                        // fixed length
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

        _deviceId1._length = payloadSize - offset;

        for (int i = 0; i < 4; ++i) {
          const uint32_t val = payloadWithoutChecksums[offset + 1 + i];
          _deviceId1._serialNr += val << (i * 8);
        }

        // We're done
        offset = payloadSize;
        break;
      default:
        // We're done
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
mBusPacket_data mBusPacket_t::removeChecksumsFrameA(const String& payload)
{
  mBusPacket_data result;
  const size_t    payloadLength = payload.length();

  if (payloadLength < 4) { return result; }

  int sourceIndex = 1; // Starts with "b"
  int targetIndex = 0;

  // 1st byte contains length of data (excuding 1st byte and excluding CRC)
  const int expectedMessageSize = hexToByte(payload, sourceIndex) + 1;
  if (payloadLength < (2* expectedMessageSize)) {
    // Not an exact check, but close enough to fail early on packets which are seriously too short.
    return result;
  }

  result.reserve(expectedMessageSize);

  while (targetIndex < expectedMessageSize) {
    // end index is start index + block size + 2 byte checksums
    int blockSize = (sourceIndex == 0) ? FRAME_FORMAT_A_FIRST_BLOCK_LENGTH : FRAME_FORMAT_A_OTHER_BLOCK_LENGTH;

    if ((targetIndex + blockSize) > expectedMessageSize) { // last block
      blockSize = expectedMessageSize - targetIndex;
    }

    // FIXME: handle truncated source messages
    for (int i = 0; i < blockSize; ++i) {
      result.push_back(hexToByte(payload, sourceIndex));
      sourceIndex += 2; // 2 hex chars
    }
    sourceIndex += 4;   // Skip 2 bytes CRC => 4 hex chars
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
mBusPacket_data mBusPacket_t::removeChecksumsFrameB(const String& payload)
{
  mBusPacket_data result;
  const size_t    payloadLength = payload.length();

  if (payloadLength < 4) { return result; }

  int sourceIndex = 2; // Starts with "bY"

  // 1st byte contains length of data (excuding 1st byte BUT INCLUDING CRC)
  int expectedMessageSize = hexToByte(payload, sourceIndex) + 1;
  if (payloadLength < (2* expectedMessageSize)) {
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

  if (expectedMessageSize > 126) {
    sourceIndex += 4; // Skip 2 bytes CRC => 4 hex chars
    int block2Size = expectedMessageSize - 127;

    if (block2Size > 124) { block2Size = 124; }

    for (int i = 0; i < block2Size; ++i) {
      result.push_back(hexToByte(payload, sourceIndex));
      sourceIndex += 2; // 2 hex chars
    }
  }

  // remove the checksums and the 1st byte from the actual message length, so that the meaning of this byte is the same as in Frame A
  result[0] = static_cast<uint8_t>((expectedMessageSize - 1) & 0xff);

  return result;
}
