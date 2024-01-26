#include "../Helpers/ESPEasy_NVS_Helper.h"

#ifdef ESP32

# include "../Helpers/StringConverter.h"


ESPEasy_NVS_Helper::~ESPEasy_NVS_Helper()
{
  _preferences.end();
}

bool ESPEasy_NVS_Helper::begin(const String& nvs_namespace, bool readOnly)
{
  return _preferences.begin(nvs_namespace.c_str(), readOnly);
}

void ESPEasy_NVS_Helper::end()
{
  _preferences.end();
}

void ESPEasy_NVS_Helper::remove(const String& key)
{
  if (_preferences.isKey(key.c_str())) {
    _preferences.remove(key.c_str());
  }
}

bool ESPEasy_NVS_Helper::getPreference(const String& key, String& value)
{
  if (!_preferences.isKey(key.c_str())) {
    return false;
  }

  value = _preferences.getString(key.c_str());

  const bool res = !value.isEmpty();

  addLog(res ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR, concat(F("NVS  : Load "), key));
  return res;
}

void ESPEasy_NVS_Helper::setPreference(const String& key, const String& value)
{
  if (value.isEmpty()) {
    _preferences.remove(key.c_str());
  }
  else if (!_preferences.getString(key.c_str(), EMPTY_STRING.c_str()).equals(value)) {
    _preferences.putString(key.c_str(), value);
    addLog(LOG_LEVEL_INFO, concat(F("NVS  : Save "), key));
  }
}

bool ESPEasy_NVS_Helper::getPreference(const String& key, uint32_t& value)
{
  if (!_preferences.isKey(key.c_str())) {
    return false;
  }
  constexpr uint32_t defaultValue = std::numeric_limits<uint32_t>::max();

  value = _preferences.getUInt(key.c_str(), defaultValue);

  const bool res = value != defaultValue;

  addLog(res ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR, concat(F("NVS  : Load "), key));
  return res;
}

void ESPEasy_NVS_Helper::setPreference(const String& key, const uint32_t& value)
{
  constexpr uint32_t defaultValue = std::numeric_limits<uint32_t>::max();

  if (value == defaultValue) {
    _preferences.remove(key.c_str());
  }
  else if (_preferences.getUInt(key.c_str()) != value) {
    _preferences.putUInt(key.c_str(), value);
    addLog(LOG_LEVEL_INFO, concat(F("NVS  : Save "), key));
  }
}

bool ESPEasy_NVS_Helper::getPreference(const String& key, uint64_t& value)
{
  if (!_preferences.isKey(key.c_str())) {
    return false;
  }

  // Make this a signed value as an erased flash chip only has 0xFF's
  // and max uint64 also contains only 0xFF bytes.
  constexpr uint64_t defaultValue = std::numeric_limits<int64_t>::max();

  value = _preferences.getULong64(key.c_str(), defaultValue);

  const bool res = value != defaultValue;

  addLog(res ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR, concat(F("NVS  : Load "), key));
  return res;
}

void ESPEasy_NVS_Helper::setPreference(const String& key, const uint64_t& value)
{
  constexpr uint64_t defaultValue = std::numeric_limits<uint64_t>::max();

  if (value == defaultValue) {
    _preferences.remove(key.c_str());
  }
  else if (_preferences.getULong64(key.c_str()) != value) {
    _preferences.putULong64(key.c_str(), value);
    addLog(LOG_LEVEL_INFO, concat(F("NVS  : Save "), key));
  }
}

bool ESPEasy_NVS_Helper::getPreference(const String& key, uint8_t *data, size_t length)
{
  if (!_preferences.isKey(key.c_str())) {
    return false;
  }

  const bool res = _preferences.getBytes(key.c_str(), data, length) == length;

  addLog(res ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR, concat(F("NVS  : Load "), key));
  return res;
}

void ESPEasy_NVS_Helper::setPreference(const String& key, const uint8_t *data, size_t length)
{
  {
    uint8_t tmp[length]{};

    if (_preferences.getBytes(key.c_str(), tmp, length) == length) {
      if (memcmp(tmp, data, length) == 0) {
        return;
      }
    }
  }

  if (_preferences.putBytes(key.c_str(), data, length) == length) {
    addLog(LOG_LEVEL_INFO, concat(F("NVS  : Save "), key));
  }
}

#endif // ifdef ESP32
