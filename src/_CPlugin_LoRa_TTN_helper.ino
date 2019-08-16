// #######################################################################################################
// #  Helper functions to encode data for use on LoRa/TTN network.
// #######################################################################################################

static void LoRa_uintToBytes(uint64_t value, uint8_t byteSize, byte *data, uint8_t& cursor) {
  // Clip values to upper limit
  const uint64_t upperlimit = (1 << (8*byteSize)) - 1;
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

static void LoRa_intToBytes(int64_t value, uint8_t byteSize, byte *data, uint8_t& cursor) {
  // Clip values to lower limit
  const int64_t lowerlimit = (1 << ((8*byteSize) - 1)) * -1;
  if (value < lowerlimit) { value = lowerlimit; }
  if (value < 0) {
    value += (1 << (8*byteSize));
  }
  LoRa_uintToBytes(value, byteSize, data, cursor);
}

static String LoRa_base16Encode(byte *data, size_t size) {
  String output;
  output.reserve(size * 2);
  char buffer[3];
  for (unsigned i=0; i<size; i++)
  {
    sprintf(buffer, "%02X", data[i]);
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
