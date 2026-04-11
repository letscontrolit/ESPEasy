#ifndef DATASTRUCTS_SHORTCHECKSUMTYPE_H
#define DATASTRUCTS_SHORTCHECKSUMTYPE_H

#include "../../ESPEasy_common.h"

// Short (4 byte) version of ChecksumType
struct __attribute__((__packed__)) ShortChecksumType {
  // Empty checksum
  ShortChecksumType() = default;

  ShortChecksumType(const ShortChecksumType& rhs);

  ShortChecksumType(uint8_t checksum[4]);

  // Construct with checksum over entire range of given data
  ShortChecksumType(const uint8_t *data,
                    size_t         data_length);

  ShortChecksumType(const uint8_t *data,
                    size_t         data_length,
                    size_t         len_upto_checksum);

  ShortChecksumType(const String strings[],
                    size_t       nrStrings);

  // Compute checksum of the data.
  // Skip the part where the checksum may be located in the data
  // @param checksum The expected checksum. Will contain checksum after call finished.
  // @retval true when checksum matches
  static bool computeChecksum(
    uint8_t        checksum[4],
    const uint8_t *data,
    size_t         data_length,
    size_t         len_upto_checksum,
    bool           updateChecksum = true);

  void               getChecksum(uint8_t checksum[4]) const;
  void               setChecksum(const uint8_t checksum[4]);
  bool               matchChecksum(const uint8_t checksum[4]) const;
  bool               operator==(const ShortChecksumType& rhs) const;
  ShortChecksumType& operator=(const ShortChecksumType& rhs);

  String             toString() const;

  bool               isSet() const;

  void               clear();

private:

  static void md5sumToShortChecksum(const uint8_t md5[16],
                                    uint8_t       shortChecksum[4]);

  uint8_t _checksum[4] = { 0 };
};

#endif // ifndef DATASTRUCTS_SHORTCHECKSUMTYPE_H
