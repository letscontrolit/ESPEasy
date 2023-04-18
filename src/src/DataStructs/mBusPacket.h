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

  uint64_t      encode_toUInt64() const;

  void          decode_fromUint64(uint64_t encodedValue);

  bool          isValid() const;

  bool          matchSerial(uint32_t serialNr) const;

  void          clear();

  // Use for stats as key:
  union {
    uint64_t _encodedValue{};
    struct {
      uint64_t _serialNr     : 32;
      uint64_t _manufacturer : 16;
      uint64_t _meterType    : 8;

      // Use for filtering
      uint64_t _length : 8;
    };
  };
};

struct mBusPacket_t {
public:

  bool                       parse(const String& payload);

  // Get the header of the actual device, not the forwarding device (if present)
  const mBusPacket_header_t* getDeviceHeader() const;

  static  int16_t            decode_LQI_RSSI(uint16_t lqi_rssi,
                                             uint8_t& LQI);

  bool                       matchSerial(uint32_t serialNr) const;

  uint32_t                   getDeviceSerial() const;

  String                     toString() const;

  uint64_t                   deviceID_toUInt64() const;

private:

  static uint8_t         hexToByte(const String& str,
                                   size_t        index);

  static mBusPacket_data removeChecksumsFrameA(const String& payload,
                                               uint32_t    & checksum);
  static mBusPacket_data removeChecksumsFrameB(const String& payload,
                                               uint32_t    & checksum);

  bool                   parseHeaders(const mBusPacket_data& payloadWithoutChecksums);

public:

  mBusPacket_header_t _deviceId1;
  mBusPacket_header_t _deviceId2;
  uint16_t            _lqi_rssi{};


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
  uint32_t _checksum = 0;
};

#endif // ifndef DATASTRUCTS_MBUSPACKET_H
