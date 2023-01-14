#include "../DataStructs/ChecksumType.h"

#include <MD5Builder.h>

ChecksumType::ChecksumType(uint8_t checksum[16])
{
  memcpy(_checksum, checksum, 16);
}

ChecksumType::ChecksumType(uint8_t *data,
                           size_t   data_length)
{
  computeChecksum(_checksum, data, data_length, data_length, true);
}

ChecksumType::ChecksumType(uint8_t *data,
                           size_t   data_length,
                           size_t   len_upto_md5)
{
  computeChecksum(_checksum, data, data_length, len_upto_md5, true);
}

bool ChecksumType::computeChecksum(
  uint8_t  checksum[16],
  uint8_t *data,
  size_t   data_length,
  size_t   len_upto_md5,
  bool     updateChecksum)
{
  if (len_upto_md5 > data_length) { len_upto_md5 = data_length; }
  MD5Builder md5;

  md5.begin();

  if (len_upto_md5 > 0) {
    md5.add(data, len_upto_md5);
  }

  if ((len_upto_md5 + 16) < data_length) {
    data += len_upto_md5 + 16;
    const int len_after_md5 = data_length - 16 - len_upto_md5;

    if (len_after_md5 > 0) {
      md5.add(data, len_after_md5);
    }
  }
  md5.calculate();
  uint8_t tmp_md5[16] = { 0 };

  md5.getBytes(tmp_md5);

  if (memcmp(tmp_md5, checksum, 16) != 0) {
    // Data has changed, copy computed checksum
    if (updateChecksum) {
      memcpy(checksum, tmp_md5, 16);
    }
    return false;
  }
  return true;
}

void ChecksumType::getChecksum(uint8_t checksum[16]) const {
  memcpy(checksum, _checksum, 16);
}

void ChecksumType::setChecksum(const uint8_t checksum[16]) {
  memcpy(_checksum, checksum, 16);
}

bool ChecksumType::matchChecksum(const uint8_t checksum[16]) const {
  return memcmp(_checksum, checksum, 16) == 0;
}
