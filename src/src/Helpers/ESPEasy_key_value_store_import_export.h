#pragma once
#include "../../ESPEasy_common.h"

#if FEATURE_ESPEASY_KEY_VALUE_STORE
# include "../DataTypes/ESPEasy_key_value_store_StorageType.h"

# include <map>

class ESPEasy_key_value_store;
class KeyValueWriter;

class ESPEasy_key_value_store_import_export
{
public:

  using LabelStringFunction = const __FlashStringHelper * (*)(uint32_t, bool, KVS_StorageType::Enum&);


  // When queried with a key of -1, it will return the first key index
  // Return next key, or -2 when no next key exists.
  using NextKeyFunction = int32_t (*)(int32_t);

  ESPEasy_key_value_store_import_export(
    ESPEasy_key_value_store*store);

  bool do_export(uint32_t            key,
             KeyValueWriter     *writer,
             LabelStringFunction fnc) const;


  ESPEasy_key_value_store_import_export(
    ESPEasy_key_value_store*store,
    const String          & json);

  String do_import(
    LabelStringFunction fnc,
    NextKeyFunction     nextKey);

  bool getParsedJSON(const String& key,
                     String      & value) const;

private:

  std::map<String, String>_parsedJSON;

  ESPEasy_key_value_store*_store{};

}; // class ESPEasy_key_value_store_import_export

#endif // if FEATURE_ESPEASY_KEY_VALUE_STORE
