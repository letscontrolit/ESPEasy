#include "../DataTypes/ESPEasy_key_value_store_data.h"

#if FEATURE_ESPEASY_KEY_VALUE_STORE

constexpr unsigned int size_32bit = sizeof(float);

ESPEasy_key_value_store_4byte_data_t::ESPEasy_key_value_store_4byte_data_t() {
  ZERO_FILL(binary);
}

ESPEasy_key_value_store_4byte_data_t::ESPEasy_key_value_store_4byte_data_t(const ESPEasy_key_value_store_4byte_data_t& other)
{
  memcpy(binary, other.binary, sizeof(binary));
}

ESPEasy_key_value_store_4byte_data_t::ESPEasy_key_value_store_4byte_data_t(const uint8_t*data)
{
  memcpy(binary, data, sizeof(binary));
}

ESPEasy_key_value_store_4byte_data_t& ESPEasy_key_value_store_4byte_data_t::operator=(const ESPEasy_key_value_store_4byte_data_t& other)
{
  memcpy(binary, other.binary, sizeof(binary));
  return *this;
}

void    ESPEasy_key_value_store_4byte_data_t::clear() { ZERO_FILL(binary); }

int32_t ESPEasy_key_value_store_4byte_data_t::getInt32() const
{
  int32_t res{};

  memcpy(&res, binary, size_32bit);
  return res;
}

bool     ESPEasy_key_value_store_4byte_data_t::setInt32(int32_t value) { return set(&value); }

uint32_t ESPEasy_key_value_store_4byte_data_t::getUint32() const
{
  uint32_t res{};

  memcpy(&res, binary, size_32bit);
  return res;
}

bool  ESPEasy_key_value_store_4byte_data_t::setUint32(uint32_t value) { return set(&value); }

float ESPEasy_key_value_store_4byte_data_t::getFloat() const
{
  float res{};

  memcpy(&res, binary, size_32bit);
  return res;
}

bool ESPEasy_key_value_store_4byte_data_t::setFloat(float value) { return set(&value); }

bool ESPEasy_key_value_store_4byte_data_t::set(const void*value)
{
  if (memcmp(binary, value, size_32bit) == 0) { return false; }
  memcpy(binary, value, size_32bit);
  return true;
}

#endif // if FEATURE_ESPEASY_KEY_VALUE_STORE
