#include "../Helpers/ESPEasy_key_value_store_import_export.h"

#if FEATURE_ESPEASY_KEY_VALUE_STORE
# include "../Helpers/_ESPEasy_key_value_store.h"
# include "../Helpers/KeyValueWriter.h"
# include "../Helpers/StringConverter.h"


ESPEasy_key_value_store_import_export::ESPEasy_key_value_store_import_export(
  ESPEasy_key_value_store*store)
  : _store(store) {}


bool ESPEasy_key_value_store_import_export::do_export(
  uint32_t       key,
  KeyValueWriter*writer,
  LabelStringFunction     fnc) const
{
  if ((_store == nullptr) || (fnc == nullptr) || (writer == nullptr)) {
    return false;
  }
  KVS_StorageType::Enum storageType = KVS_StorageType::Enum::not_set;

  auto label = fnc(key, false, storageType);

  if (!_store->hasKey(storageType, key)) {
    return false;
  }

  switch (storageType)
  {
    case KVS_StorageType::Enum::not_set:
    case KVS_StorageType::Enum::MAX_Type:
      break;
    case KVS_StorageType::Enum::binary:
      // TODO TD-er: Implement
      break;
    case KVS_StorageType::Enum::bool_type:
    case KVS_StorageType::Enum::bool_true:
    case KVS_StorageType::Enum::bool_false:
    {
      bool value{};

      if (_store->getValue(key, value)) {
        writer->write({ label, value });
        return true;
      }
      break;
    }
    case KVS_StorageType::Enum::float_type:
    {
      float value{};

      if (_store->getValue(key, value)) {
        writer->write({ label, value });
        return true;
      }
      break;
    }
    case KVS_StorageType::Enum::double_type:
    {
      double value{};

      if (_store->getValue(key, value)) {
        writer->write({ label, value });
        return true;
      }
      break;
    }
    default:
    {
      String value;

      if (_store->getValueAsString(key, value)) {
        writer->write({ label, value });
        return true;
      }
      break;
    }
  }
  return false;
}


bool getNextKeyValue(String& json, String& key, String& value)
{
  String keyValueStr;
  int pos = json.indexOf(',');
  if (pos == -1) keyValueStr = json;
  else keyValueStr = json.substring(0, pos);

  if (keyValueStr.isEmpty()) {
    return false;
  }

  // KeyValueStr now contains something like this:
  // "key": value
  // "key": "value"

  key   = parseStringKeepCase(keyValueStr, 1, ':');
  value = parseStringKeepCase(keyValueStr, 2, ':');

  addLog(LOG_LEVEL_INFO, strformat(F("KVS : kvs: '%s' key:'%s' value:'%s'"), keyValueStr.c_str(), key.c_str(), value.c_str()));

  // Strip found item off
  json = json.substring(keyValueStr.length());
  json.trim();

  if (json.startsWith(",")) {
    json = json.substring(1);
  }
  return true;
}


ESPEasy_key_value_store_import_export::ESPEasy_key_value_store_import_export(
  ESPEasy_key_value_store*store, const String& json)
  : _store(store)
{
  if ((_store != nullptr) && _store->isEmpty()) {
    String tmp_json(json);

    tmp_json.trim();

    if (tmp_json.startsWith("{") && tmp_json.endsWith("}")) {
      tmp_json = tmp_json.substring(1, tmp_json.length() - 1);
    }
    tmp_json.trim();

    String key_str;
    String value;

    while (getNextKeyValue(tmp_json, key_str, value)) {
      key_str.toLowerCase();
      _parsedJSON[key_str] = value;
    }
  }
}




String ESPEasy_key_value_store_import_export::do_import(
  LabelStringFunction fnc,
  NextKeyFunction     nextKey)
{
  if (_store == nullptr) {
    return F("No Store Set");
  }

  if (!_store->isEmpty()) {
    return F("Store not empty");
  }
  fnc = fnc;

  if ((fnc == nullptr) || (nextKey == nullptr)) {
    return F("No decoder functions set");
  }

  int32_t key = nextKey(-1);

  while (key >= 0) {
    KVS_StorageType::Enum storageType = KVS_StorageType::Enum::not_set;

    auto label = fnc(key, false, storageType);

    String value;

    if (getParsedJSON(label, value)) {
      _store->setValue(storageType, key, value);
    }
    key = nextKey(key);
  }

  _store->dump();
  return EMPTY_STRING;
}

bool ESPEasy_key_value_store_import_export::getParsedJSON(const String& key, String& value) const
{
  String key_lc = key;

  key_lc.toLowerCase();
  auto it = _parsedJSON.find(key_lc);

  if (it == _parsedJSON.end()) { return false; }
  value = it->second;
  return true;
}

#endif // if FEATURE_ESPEASY_KEY_VALUE_STORE
