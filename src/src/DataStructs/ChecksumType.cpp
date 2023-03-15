#include "../DataStructs/ChecksumType.h"

#include "../Helpers/StringConverter.h"

#include <MD5Builder.h>

ChecksumType::ChecksumType(uint8_t checksum[16])
{
  memcpy(_checksum, checksum, 16);
}

ChecksumType::ChecksumType(const uint8_t *data,
                           size_t   data_length)
{
  computeChecksum(_checksum, data, data_length, data_length, true);
}

ChecksumType::ChecksumType(const uint8_t *data,
                           size_t   data_length,
                           size_t   len_upto_md5)
{
  computeChecksum(_checksum, data, data_length, len_upto_md5, true);
}

ChecksumType::ChecksumType(const String strings[], size_t nrStrings)
{
  MD5Builder md5;

  md5.begin();

  for (size_t i = 0; i < nrStrings; ++i) {
    md5.add(strings[i].c_str());
  }
  md5.calculate();
  md5.getBytes(_checksum);
}

bool ChecksumType::computeChecksum(
  uint8_t  checksum[16],
  const uint8_t *data,
  size_t   data_length,
  size_t   len_upto_md5,
  bool     updateChecksum)
{
  if (len_upto_md5 > data_length) { len_upto_md5 = data_length; }
  MD5Builder md5;

  md5.begin();

  if (len_upto_md5 > 0) {
    // MD5Builder::add has non-const argument
    md5.add(const_cast<uint8_t *>(data), len_upto_md5);
  }

  if ((len_upto_md5 + 16) < data_length) {
    data += len_upto_md5 + 16;
    const int len_after_md5 = data_length - 16 - len_upto_md5;

    if (len_after_md5 > 0) {
      // MD5Builder::add has non-const argument
      md5.add(const_cast<uint8_t *>(data), len_after_md5);
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

bool ChecksumType::operator==(const ChecksumType& rhs) const {
  return memcmp(_checksum, rhs._checksum, 16) == 0;
}

ChecksumType& ChecksumType::operator=(const ChecksumType& rhs) {
  memcpy(_checksum, rhs._checksum, 16);
  return *this;
}

String ChecksumType::toString() const {
  return formatToHex_array(_checksum, 16);
}