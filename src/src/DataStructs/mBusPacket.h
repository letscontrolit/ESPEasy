#ifndef DATASTRUCTS_MBUSPACKET_H
#define DATASTRUCTS_MBUSPACKET_H

#include "../../ESPEasy_common.h"

#include <vector>

typedef std::vector<uint8_t> mBusPacket_data;

struct mBusPacket_header_t {
  static String decodeManufacturerID(int id);
  static int    encodeManufacturerID(const String& id_str);

  String        getManufacturerId() const;

  String        toString() const;

  bool          isValid() const;

  void          clear();

  // Use for stats as key:
  uint32_t _serialNr     = 0;
  uint16_t _manufacturer = 0;
  uint8_t  _meterType    = 0;

  // Use for filtering
  uint8_t  _length       = 0;
};

struct mBusPacket_t {
public:

  bool parse(const String& payload);

private:

  static uint8_t         hexToByte(const String& str,
                                   size_t        index);

  static mBusPacket_data removeChecksumsFrameA(const String& payload, uint32_t& checksum);
  static mBusPacket_data removeChecksumsFrameB(const String& payload, uint32_t& checksum);

  bool                   parseHeaders(const mBusPacket_data& payloadWithoutChecksums);

public:

  mBusPacket_header_t _deviceId1;
  mBusPacket_header_t _deviceId2;
  int16_t             _rssi = 0;
  uint8_t             _LQI  = 0;


/*
  // Statistics:
  // Key:
  deviceID1:
  - manufacturer
  - metertype   
  - serialnr

  // Value:
  - message count
  - rssi
  - lqi???
*/


  // Checksum based on the XOR of all removed checksums from the message
  uint32_t            _checksum = 0;
};

#endif // ifndef DATASTRUCTS_MBUSPACKET_H
