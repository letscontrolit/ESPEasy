// #######################################################################################################
// #  Helper functions to encode data for use on LoRa/TTN network.
// #######################################################################################################

static void LoRa_uintToBytes(uint64_t value, uint8_t byteSize, byte *data, uint8_t& cursor) {
  for (uint8_t x = 0; x < byteSize; x++) {
    byte next = 0;
    if (sizeof(value) > x) {
      next = static_cast<byte>((value >> (x * 8)) & 0xFF);
    }
    data[cursor] = next;
    ++cursor;
  }
}

static void LoRa_floatToBytes(float value, float factor, uint8_t byteSize, byte *data, uint8_t& cursor) {
  int64_t intval = value * factor;
  if (intval < 0.0) {
    intval += (1 << (8*byteSize));
  }
  LoRa_uintToBytes(intval, byteSize, data, cursor);
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
  unsigned long factor;
  uint8_t byteSize = getPackedDataTypeSize(datatype, factor);
  byte data[4] = {0};
  uint8_t cursor = 0;
  LoRa_uintToBytes(value * factor, byteSize, &data[0], cursor);
  return LoRa_base16Encode(data, cursor);
}


String LoRa_addFloat(float value, PackedData_enum datatype) {
  unsigned long factor;
  uint8_t byteSize = getPackedDataTypeSize(datatype, factor);
  byte data[4] = {0};
  uint8_t cursor = 0;
  LoRa_floatToBytes(value, factor, byteSize, &data[0], cursor);
  return LoRa_base16Encode(data, cursor);
}
