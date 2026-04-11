#include "../DataStructs/ShortChecksumType.h"

#include "../Helpers/StringConverter.h"

#include <MD5Builder.h>

void ShortChecksumType::md5sumToShortChecksum(const uint8_t md5[16], uint8_t shortChecksum[4])
{
  memset(shortChecksum, 0, 4);

  for (uint8_t i = 0; i < 16; ++i) {
    // shortChecksum is XOR per 32 bit
    shortChecksum[i % 4] ^= md5[i];
  }
}

ShortChecksumType::ShortChecksumType(const ShortChecksumType& rhs)
{
  memcpy(_checksum, rhs._checksum, 4);
}

ShortChecksumType::ShortChecksumType(uint8_t checksum[4])
{
  memcpy(_checksum, checksum, 4);
}

ShortChecksumType::ShortChecksumType(const uint8_t *data,
                                     size_t         data_length)
{
  computeChecksum(_checksum, data, data_length, data_length, true);
}

ShortChecksumType::ShortChecksumType(const uint8_t *data,
                                     size_t         data_length,
                                     size_t         len_upto_checksum)
{
  computeChecksum(_checksum, data, data_length, len_upto_checksum, true);
}

ShortChecksumType::ShortChecksumType(const String strings[], size_t nrStrings)
{
  MD5Builder md5;

  md5.begin();

  for (size_t i = 0; i < nrStrings; ++i) {
    md5.add(strings[i].c_str());
  }
  md5.calculate();
  uint8_t tmp_md5[16] = { 0 };

  md5.getBytes(tmp_md5);
  md5sumToShortChecksum(tmp_md5, _checksum);
}

bool ShortChecksumType::computeChecksum(
  uint8_t        checksum[4],
  const uint8_t *data,
  size_t         data_length,
  size_t         len_upto_checksum,
  bool           updateChecksum)
{
  if (len_upto_checksum > data_length) { len_upto_checksum = data_length; }
  MD5Builder md5;

  md5.begin();

  if (len_upto_checksum > 0) {
    // MD5Builder::add has non-const argument
    md5.add(const_cast<uint8_t *>(data), len_upto_checksum);
  }

  if ((len_upto_checksum + 4) < data_length) {
    data += len_upto_checksum + 4;
    const int len_after_checksum = data_length - 4 - len_upto_checksum;

    if (len_after_checksum > 0) {
      // MD5Builder::add has non-const argument
      md5.add(const_cast<uint8_t *>(data), len_after_checksum);
    }
  }
  md5.calculate();
  uint8_t tmp_checksum[4] = { 0 };

  {
    uint8_t tmp_md5[16] = { 0 };
    md5.getBytes(tmp_md5);
    md5sumToShortChecksum(tmp_md5, tmp_checksum);
  }

  if (memcmp(tmp_checksum, checksum, 4) != 0) {
    // Data has changed, copy computed checksum
    if (updateChecksum) {
      memcpy(checksum, tmp_checksum, 4);
    }
    return false;
  }
  return true;
}

void ShortChecksumType::getChecksum(uint8_t checksum[4]) const {
  memcpy(checksum, _checksum, 4);
}

void ShortChecksumType::setChecksum(const uint8_t checksum[4]) {
  memcpy(_checksum, checksum, 4);
}

bool ShortChecksumType::matchChecksum(const uint8_t checksum[4]) const {
  return memcmp(_checksum, checksum, 4) == 0;
}

bool ShortChecksumType::operator==(const ShortChecksumType& rhs) const {
  return memcmp(_checksum, rhs._checksum, 4) == 0;
}

ShortChecksumType& ShortChecksumType::operator=(const ShortChecksumType& rhs) {
  memcpy(_checksum, rhs._checksum, 4);
  return *this;
}

String ShortChecksumType::toString() const {
  return formatToHex_array(_checksum, 4);
}

bool ShortChecksumType::isSet() const {
  return
    _checksum[0] != 0 ||
    _checksum[1] != 0 ||
    _checksum[2] != 0 ||
    _checksum[3] != 0;
}

void ShortChecksumType::clear()
{
  memset(_checksum, 0, 4);
}
