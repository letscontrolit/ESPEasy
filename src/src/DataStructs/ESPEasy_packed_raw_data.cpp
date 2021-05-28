#include "../DataStructs/ESPEasy_packed_raw_data.h"


uint8_t getPackedDataTypeSize(PackedData_enum dtype, float& factor, float& offset) {
  offset = 0;
  factor = 1;
  if (dtype > 0x1000 && dtype < 0x12FF) {
    const uint32_t exponent = dtype & 0xF;
    switch(exponent) {
      case 0: factor = 1; break;
      case 1: factor = 1e1; break;
      case 2: factor = 1e2; break;
      case 3: factor = 1e3; break;
      case 4: factor = 1e4; break;
      case 5: factor = 1e5; break;
      case 6: factor = 1e6; break;
    }
    const uint8_t size = (dtype >> 4) & 0xF;
    return size;
  }
  switch (dtype) {
    case PackedData_pluginid:    factor = 1;         return 1;
    case PackedData_latLng:      factor = 46600;     return 3; // 2^23 / 180
    case PackedData_hdop:        factor = 10;        return 1;
    case PackedData_altitude:    factor = 4;     offset = 1000; return 2; // -1000 .. 15383.75 meter
    case PackedData_vcc:         factor = 41.83; offset = 1;    return 1; // -1 .. 5.12V
    case PackedData_pct_8:       factor = 2.56;                 return 1; // 0 .. 100%
    default:
      break;
  }

  // Unknown type
  factor = 1;
  return 0;
}

void LoRa_uintToBytes(uint64_t value, uint8_t byteSize, byte *data, uint8_t& cursor) {
  // Clip values to upper limit
  const uint64_t upperlimit = (1ull << (8*byteSize)) - 1;
  if (value > upperlimit) { value = upperlimit; }
  for (uint8_t x = 0; x < byteSize; x++) {
    byte next = 0;
    if (sizeof(value) > x) {
      next = static_cast<byte>((value >> (x * 8)) & 0xFF);
    }
    data[cursor] = next;
    ++cursor;
  }
}

void LoRa_intToBytes(int64_t value, uint8_t byteSize, byte *data, uint8_t& cursor) {
  // Clip values to lower limit
  const int64_t lowerlimit = (1ull << ((8*byteSize) - 1)) * -1;
  if (value < lowerlimit) { value = lowerlimit; }
  if (value < 0) {
    value += (1ull << (8*byteSize));
  }
  LoRa_uintToBytes(value, byteSize, data, cursor);
}

String LoRa_base16Encode(byte *data, size_t size) {
  String output;
  output.reserve(size * 2);
  char buffer[3];
  for (unsigned i=0; i<size; i++)
  {
    sprintf_P(buffer, PSTR("%02X"), data[i]);
    output += buffer[0];
    output += buffer[1];
  }
  return output;
}

String LoRa_addInt(uint64_t value, PackedData_enum datatype) {
  float factor, offset;
  uint8_t byteSize = getPackedDataTypeSize(datatype, factor, offset);
  byte data[4] = {0};
  uint8_t cursor = 0;
  LoRa_uintToBytes((value + offset) * factor, byteSize, &data[0], cursor);
  return LoRa_base16Encode(data, cursor);
}


String LoRa_addFloat(float value, PackedData_enum datatype) {
  float factor, offset;
  uint8_t byteSize = getPackedDataTypeSize(datatype, factor, offset);
  byte data[4] = {0};
  uint8_t cursor = 0;
  LoRa_intToBytes((value + offset) * factor, byteSize, &data[0], cursor);
  return LoRa_base16Encode(data, cursor);
}
