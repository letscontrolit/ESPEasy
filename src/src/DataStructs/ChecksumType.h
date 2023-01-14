#ifndef DATASTRUCTS_CHECKSUMTYPE_H
#define DATASTRUCTS_CHECKSUMTYPE_H

#include "../../ESPEasy_common.h"

struct ChecksumType {
  // Empty checksum
  ChecksumType() = default;

  ChecksumType(uint8_t checksum[16]);

  // Construct with checksum over entire range of given data
  ChecksumType(uint8_t *data,
               size_t   data_length);

  ChecksumType(uint8_t *data,
               size_t   data_length,
               size_t   len_upto_md5);

  // Compute checksum of the data.
  // Skip the part where the checksum may be located in the data
  // @param checksum The expected checksum. Will contain checksum after call finished.
  // @retval true when checksum matches
  static bool computeChecksum(
    uint8_t  checksum[16],
    uint8_t *data,
    size_t   data_length,
    size_t   len_upto_md5,
    bool     updateChecksum = true);

  void getChecksum(uint8_t checksum[16]) const;
  void setChecksum(const uint8_t checksum[16]);
  bool matchChecksum(const uint8_t checksum[16]) const;

private:

  uint8_t _checksum[16] = { 0 };
};

#endif // ifndef DATASTRUCTS_CHECKSUMTYPE_H
