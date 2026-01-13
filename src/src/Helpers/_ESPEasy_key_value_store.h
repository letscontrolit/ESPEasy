#ifndef HELPERS_ESPEASY_KEY_VALUE_STORE_H
#define HELPERS_ESPEASY_KEY_VALUE_STORE_H

#include "../../ESPEasy_common.h"

#if FEATURE_ESPEASY_KEY_VALUE_STORE

# include "../DataTypes/ESPEasy_key_value_store_data.h"
# include "../DataTypes/ESPEasy_key_value_store_StorageType.h"

# include "../DataTypes/SettingsType.h"

# include <map>

class KeyValueWriter;

// Generic storage layer, which on a low level will store everything as a string.
// Low level storage structure:
// - uint8_t:    version of storage layer
// - uint8_t[2]: ID to match, if not matched, then stop loading
// - uint8_t[3]: rawSize (offset to start of checksum)
// - List of key/value pairs:
//   - uint8_t:    storage type
//   - uint8_t[3]: key
//   - N bytes value; either:
//     - String: 2 bytes length + string (without 0-termination)
//     - shortened binary form
//   ...
// - uint8_t[16] checksum

class ESPEasy_key_value_store
{
public:

  typedef std::pair<String, String> StringPair;

  enum class State {
    Empty,
    NotChanged, // Not changed since last load/save
    Changed,
    ErrorOnLoad,
    ErrorOnSave

  };

  State                               getState() const { return _state; }

  bool                                isEmpty() const;

  void                                clear();


  bool                                load(SettingsType::Enum settingsType,
                                           int                index,
                                           uint32_t           offset_in_block,
                                           uint16_t           id_to_match);
  bool store(SettingsType::Enum settingsType,
             int                index,
             uint32_t           offset_in_block,
             uint16_t           id_to_match);

  // Count all data to estimate how much storage space it would require to store everything in a somewhat compact form.
  size_t getPayloadStorageSize() const;

  // Check to see if there is a key stored with given storage type
  bool   hasKey(KVS_StorageType::Enum storageType,
                uint32_t                     key) const;

  // Try to find a key and get its storage type or 'not_set' if the key is not present
  KVS_StorageType::Enum getStorageType(uint32_t key) const;


  /*
   ###############################################
   ## get/set functions for all supported types ##
   ###############################################
   */

  bool getValue(uint32_t    key,
                StringPair& stringPair) const;
  void setValue(uint32_t          key,
                const StringPair& stringPair);

  bool getValue(uint32_t key,
                String & value) const;
  void setValue(uint32_t      key,
                const String& value);
  void setValue(uint32_t                  key,
                const __FlashStringHelper*value);
  void setValue(uint32_t key,
                String&& value);

  bool getValue(uint32_t key,
                int8_t & value) const;
  void setValue(uint32_t      key,
                const int8_t& value);

  bool getValue(uint32_t key,
                uint8_t& value) const;
  void setValue(uint32_t       key,
                const uint8_t& value);

  bool getValue(uint32_t key,
                int16_t& value) const;
  void setValue(uint32_t       key,
                const int16_t& value);

  bool getValue(uint32_t  key,
                uint16_t& value) const;
  void setValue(uint32_t        key,
                const uint16_t& value);

  bool getValue(uint32_t key,
                int32_t& value) const;
  void setValue(uint32_t       key,
                const int32_t& value);

  bool getValue(uint32_t  key,
                uint32_t& value) const;
  void setValue(uint32_t        key,
                const uint32_t& value);

  bool getValue(uint32_t key,
                int64_t& value) const;
  void setValue(uint32_t       key,
                const int64_t& value);

  bool getValue(uint32_t  key,
                uint64_t& value) const;
  void setValue(uint32_t        key,
                const uint64_t& value);

  bool getValue(uint32_t key,
                float  & value) const;
  void setValue(uint32_t     key,
                const float& value);

  bool getValue(uint32_t key,
                double & value) const;
  void setValue(uint32_t      key,
                const double& value);

  bool getValue(uint32_t key,
                bool   & value) const;
  void setValue(uint32_t    key,
                const bool& value);


  // Generic get function for any given storageType/key and represent its value as a string.
  // Return false when storageType/key is not present.
  bool getValueAsString(const KVS_StorageType::Enum& storageType,
                        uint32_t                            key,
                        String                            & value) const;

  bool    getValueAsString(uint32_t key,
                           String & value) const;

  bool    getValueAsInt(uint32_t key,
                        int64_t& value) const;

  int64_t getValueAsInt_or_default(uint32_t key,
                                   int64_t  default_value) const;
  int64_t getValueAsInt(uint32_t key) const;

  // Generic set function for any given storageType/key.
  // Given value is a string representation of that storage type.
  // TODO TD-er: Implement
  void setValue(const KVS_StorageType::Enum& storageType,
                uint32_t                            key,
                const String                      & value);

  String getLastError() const { return _lastError; }

  void   dump() const;

private:

  bool getValue(KVS_StorageType::Enum        & storageType,
                uint32_t                              key,
                ESPEasy_key_value_store_4byte_data_t& value) const;

  void setValue(KVS_StorageType::Enum              & storageType,
                uint32_t                                    key,
                const ESPEasy_key_value_store_4byte_data_t& value);

  bool getValue(KVS_StorageType::Enum        & storageType,
                uint32_t                              key,
                ESPEasy_key_value_store_8byte_data_t& value) const;

  void setValue(KVS_StorageType::Enum              & storageType,
                uint32_t                                    key,
                const ESPEasy_key_value_store_8byte_data_t& value);


  // Query cache to see if we have any of the asked storage type
  bool hasStorageType(KVS_StorageType::Enum storageType) const;

  // Update cache to indicate we have at least one of the given storage type
  void setHasStorageType(KVS_StorageType::Enum storageType);

  typedef std::map<uint32_t, ESPEasy_key_value_store_4byte_data_t> map_4byte_data;
  typedef std::map<uint32_t, ESPEasy_key_value_store_8byte_data_t> map_8byte_data;

  map_4byte_data::const_iterator get4byteIterator(
    KVS_StorageType::Enum storageType,
    uint32_t                     key) const;
  map_8byte_data::const_iterator get8byteIterator(
    KVS_StorageType::Enum storageType,
    uint32_t                     key) const;

  String _lastError;

  std::map<uint32_t, String>_string_data{};
  map_4byte_data _4byte_data{};
  map_8byte_data _8byte_data{};

  uint32_t _storage_type_present_cache{};

  // TODO TD-er: Add checksum and invalidate whenever anyting is being stored.
  State _state{ State::Empty };

}; // class ESPEasy_key_value_store

#endif // if FEATURE_ESPEASY_KEY_VALUE_STORE

#endif // ifndef HELPERS_ESPEASY_KEY_VALUE_STORE_H
