#include "../DataTypes/ESPEasy_key_value_store_StorageType.h"
#if FEATURE_ESPEASY_KEY_VALUE_STORE

uint32_t KVS_StorageType::combine_StorageType_and_key(
  KVS_StorageType::Enum storageType,
  uint32_t              key)
{
  uint32_t res = key & ((1 << 24) - 1);

  res |= (static_cast<uint32_t>(storageType) << 24);
  return res;
}

KVS_StorageType::Enum KVS_StorageType::get_StorageType_from_combined_key(uint32_t combined_key)
{
  return static_cast<KVS_StorageType::Enum>(combined_key >> 24);
}

uint32_t KVS_StorageType::getKey_from_combined_key(uint32_t combined_key)
{
  const uint32_t res = combined_key & ((1 << 24) - 1);

  return res;
}

size_t KVS_StorageType::getStorageSizePerType(KVS_StorageType::Enum storageType)
{
  switch (storageType)
  {
    case KVS_StorageType::Enum::not_set:      break;
    case KVS_StorageType::Enum::string_type:  break;
    case KVS_StorageType::Enum::int8_type:    return 4 + 1;
    case KVS_StorageType::Enum::uint8_type:   return 4 + 1;
    case KVS_StorageType::Enum::int16_type:   return 4 + 2;
    case KVS_StorageType::Enum::uint16_type:  return 4 + 2;
    case KVS_StorageType::Enum::int32_type:   return 4 + 4;
    case KVS_StorageType::Enum::uint32_type:  return 4 + 4;
    case KVS_StorageType::Enum::int64_type:   return 4 + 8;
    case KVS_StorageType::Enum::uint64_type:  return 4 + 8;
    case KVS_StorageType::Enum::float_type:   return 4 + 4;
    case KVS_StorageType::Enum::double_type:  return 4 + 8;
    case KVS_StorageType::Enum::bool_type:    return 4 + 0;
    case KVS_StorageType::Enum::MAX_Type:     break;
    case KVS_StorageType::Enum::bool_true:
    case KVS_StorageType::Enum::bool_false:   return 4 + 0;

    case KVS_StorageType::Enum::binary: break;
  }
  return 0;
}

#endif // if FEATURE_ESPEASY_KEY_VALUE_STORE
