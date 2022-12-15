#ifndef DATASTRUCTS_MBUSPACKET_H
#define DATASTRUCTS_MBUSPACKET_H

#include "../../ESPEasy_common.h"

#include <vector>

typedef std::vector<uint8_t> mBusPacket_data;

struct mBusPacket_header_t {
  static String decodeManufacturerID(int id);
  static int    encodeManufacturerID(const String& id_str);

  String        getManufacturerId() const;

  String toString() const;

  bool isValid() const;

  int      _manufacturer = 0;
  int      _meterType    = 0;
  uint32_t _serialNr     = 0;
  int      _length       = 0;
};

struct mBusPacket_t {
public:

  bool parse(const String& payload);

private:

  static uint8_t         hexToByte(const String& str,
                                   size_t        index);

  static mBusPacket_data removeChecksumsFrameA(const String& payload);
  static mBusPacket_data removeChecksumsFrameB(const String& payload);

  bool                   parseHeaders(const mBusPacket_data& payloadWithoutChecksums);

public:

  mBusPacket_header_t _deviceId1;
  mBusPacket_header_t _deviceId2;
};

#endif // ifndef DATASTRUCTS_MBUSPACKET_H
