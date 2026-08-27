#ifndef DATATYPES_ESPEASY_KEY_VALUE_STORE_DATA_H
#define DATATYPES_ESPEASY_KEY_VALUE_STORE_DATA_H

#include "../../ESPEasy_common.h"

#if FEATURE_ESPEASY_KEY_VALUE_STORE

// Store various types of 32 bit
class alignas(uint32_t) ESPEasy_key_value_store_4byte_data_t
{
public:

  ESPEasy_key_value_store_4byte_data_t();

  ESPEasy_key_value_store_4byte_data_t(const ESPEasy_key_value_store_4byte_data_t& other);

  ESPEasy_key_value_store_4byte_data_t(const uint8_t*data);

  ESPEasy_key_value_store_4byte_data_t& operator=(const ESPEasy_key_value_store_4byte_data_t& other);

  void                                  clear();


  int32_t                               getInt32() const;

  // Return true when content has changed
  bool                                  setInt32(int32_t value);

  uint32_t                              getUint32() const;

  // Return true when content has changed
  bool                                  setUint32(uint32_t value);

  float                                 getFloat() const;

  // Return true when content has changed
  bool                                  setFloat(float value);

  uint8_t*                              getBinary()       { return binary; }

  const uint8_t*                        getBinary() const { return binary; }

  // Return true when content has changed
  bool                                  set(const void*value);

private:

  uint8_t binary[sizeof(float)]{};
}; // class alignas

// Store various types of 64 bit
class alignas(uint32_t) ESPEasy_key_value_store_8byte_data_t
{
public:

  ESPEasy_key_value_store_8byte_data_t();

  ESPEasy_key_value_store_8byte_data_t(const ESPEasy_key_value_store_8byte_data_t& other);

  ESPEasy_key_value_store_8byte_data_t(const uint8_t*data);

  ESPEasy_key_value_store_8byte_data_t& operator=(const ESPEasy_key_value_store_8byte_data_t& other);

  void                                  clear();


  int64_t                               getInt64() const;

  // Return true when content has changed
  bool                                  setInt64(int64_t value);

  uint64_t                              getUint64() const;

  // Return true when content has changed
  bool                                  setUint64(uint64_t value);

  double                                getDouble() const;

  // Return true when content has changed
  bool                                  setDouble(double value);

  uint8_t*                              getBinary()       { return binary; }

  const uint8_t*                        getBinary() const { return binary; }


  // Return true when content has changed
  bool set(const void*value);

private:

  uint8_t binary[sizeof(double)]{};
}; // class alignas

#endif // if FEATURE_ESPEASY_KEY_VALUE_STORE
#endif // ifndef DATATYPES_ESPEASY_KEY_VALUE_STORE_DATA_H
