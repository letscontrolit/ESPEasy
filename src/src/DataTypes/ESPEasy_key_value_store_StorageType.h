#pragma once

#include "../../ESPEasy_common.h"
#if FEATURE_ESPEASY_KEY_VALUE_STORE

class KVS_StorageType
{

public:

  // Type is stored, so do not change the order/values
  enum class Enum : uint8_t {
    not_set     = 0,
    string_type = 1,
    int8_type   = 2,
    uint8_type  = 3,
    int16_type  = 4,
    uint16_type = 5,
    int32_type  = 6,
    uint32_type = 7,
    int64_type  = 8,
    uint64_type = 9,
    float_type  = 10,
    double_type = 11,
    bool_type   = 12,

    MAX_Type,         // Leave this one after the generic type and before 'special' types


    bool_true  = 126, // small optimization to store 'true' as extra type
    bool_false = 127, // small optimization to store 'false' as extra type
    binary     = 255

  };

  static uint32_t combine_StorageType_and_key(
    KVS_StorageType::Enum storageType,
    uint32_t              key);

  static KVS_StorageType::Enum get_StorageType_from_combined_key(
    uint32_t combined_key);

  static uint32_t              getKey_from_combined_key(uint32_t combined_key);

  static size_t                getStorageSizePerType(KVS_StorageType::Enum storageType);


}; // class KVS_StorageType

#endif // if FEATURE_ESPEASY_KEY_VALUE_STORE
