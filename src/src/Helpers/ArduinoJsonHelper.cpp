#include "../Helpers/ArduinoJsonHelper.h"

ArduinoJsonHelper_t::ArduinoJsonHelper_t() :
  root(nullptr), _jsonLength(0)
{}

ArduinoJsonHelper_t::ArduinoJsonHelper_t(size_t jsonLength) : _jsonLength(jsonLength)
{
  root = new (std::nothrow) DynamicJsonDocument(jsonLength);
}

ArduinoJsonHelper_t::~ArduinoJsonHelper_t()
{
  if (nullptr != root) {
    // No need to call root->clear()
    // See: https://arduinojson.org/v6/api/jsondocument/clear/
    delete root;
    root = nullptr;
  }
}

void ArduinoJsonHelper_t::clear()
{
  if (nullptr != root) {
    root->clear();
  }
}

bool ArduinoJsonHelper_t::resetMemoryPool(size_t jsonLength)
{
  if (nullptr != root) {
    // No need to call root->clear()
    // See: https://arduinojson.org/v6/api/jsondocument/clear/
    delete root;
    root        = nullptr;
    _jsonLength = 0;
  }

  if (jsonLength == 0) {
    return false;
  }

  root = new (std::nothrow) DynamicJsonDocument(jsonLength);

  if (nullptr == root) {
    return false;
  }
  _jsonLength = jsonLength;
  return true;
}

DeserializationError ArduinoJsonHelper_t::do_deserializeJson(const String& json)
{
  if (root != nullptr) {
    return deserializeJson(*root, json);
  }
  const DeserializationError error(DeserializationError::Code::NoMemory);

  return error;
}

bool ArduinoJsonHelper_t::isNull() const
{
  if (root == nullptr) { return true; }
  return root->isNull();
}

int ArduinoJsonHelper_t::getInt(const __FlashStringHelper *key) const
{
  return getInt(String(key));
}

int ArduinoJsonHelper_t::getInt(const String& key) const
{
  int res{};

  if (root != nullptr) {
    res = (*root)[key];
  }
  return res;
}

float ArduinoJsonHelper_t::getFloat(const __FlashStringHelper *key) const
{
  return getFloat(String(key));
}

float ArduinoJsonHelper_t::getFloat(const String& key) const
{
  float res{};

  if (root != nullptr) {
    res = (*root)[key];
  }
  return res;
}

String ArduinoJsonHelper_t::getString(const __FlashStringHelper *key, const __FlashStringHelper *default_str) const
{
  return getString(String(key), String(default_str));
}

String ArduinoJsonHelper_t::getString(const __FlashStringHelper *key, const String& default_str) const
{
  return getString(String(key), default_str);
}

String ArduinoJsonHelper_t::getString(const String& key, const String& default_str) const
{
  const char *res = nullptr;

  if (root != nullptr) {
    res = (*root)[key];
  }

  if (res == nullptr) {
    return default_str;
  }
  return String(res);
}

JsonObject ArduinoJsonHelper_t::getDoc()
{
  if (root != nullptr) {
    return root->as<JsonObject>();
  }

  // Return empty object
  JsonObject res;

  return res;
}
