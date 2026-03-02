#include "../DataStructs/KeyValueStruct.h"

#include "../Helpers/ESPEasy_UnitOfMeasure.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringConverter_Numerical.h"

#include "../WebServer/Markup.h"


// ********************************************************************************
// KeyValueStruct
// ********************************************************************************

KeyValueStruct::KeyValueStruct(const __FlashStringHelper *key, Format format) : _key(key), _format(format) {}

KeyValueStruct::KeyValueStruct(const String& key, Format format) : _key(key), _format(format) {}


KeyValueStruct::KeyValueStruct(const String& key,
                               const bool  & val,
                               Format        format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

KeyValueStruct::KeyValueStruct(const String& key,
                               int           val,
                               Format        format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

#if defined(ESP32) && !defined(__riscv)
KeyValueStruct::KeyValueStruct(const String& key,
                               int32_t       val,
                               Format        format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

#endif // if defined(ESP32) && !defined(__riscv)
KeyValueStruct::KeyValueStruct(const String& key,
                               uint32_t      val,
                               Format        format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

#if defined(ESP32) && !defined(__riscv)
KeyValueStruct::KeyValueStruct(const String& key,
                               size_t        val,
                               Format        format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

#endif // if defined(ESP32) && !defined(__riscv)
KeyValueStruct::KeyValueStruct(const String  & key,
                               const uint64_t& val,
                               Format          format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

KeyValueStruct::KeyValueStruct(const String & key,
                               const int64_t& val,
                               Format         format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

KeyValueStruct::KeyValueStruct(const String& key,
                               const float & val,
                               uint8_t       nrDecimals,
                               Format        format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val, nrDecimals));
}

KeyValueStruct::KeyValueStruct(const String& key,
                               const double& val,
                               uint8_t       nrDecimals,
                               Format        format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val, nrDecimals));
}

KeyValueStruct::KeyValueStruct(const __FlashStringHelper *key,
                               const String             & val,
                               Format                     format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

KeyValueStruct::KeyValueStruct(const String             & key,
                               const __FlashStringHelper *val,
                               Format                     format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

KeyValueStruct::KeyValueStruct(const __FlashStringHelper *key,
                               const __FlashStringHelper *val,
                               Format                     format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

KeyValueStruct::KeyValueStruct(const __FlashStringHelper *key,
                               const char                *val,
                               Format                     format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(String(val)));
}

KeyValueStruct::KeyValueStruct(const String& key,
                               const String& val,
                               Format        format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

KeyValueStruct::KeyValueStruct(const __FlashStringHelper *key,
                               String                  && val,
                               Format                     format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(std::move(val)));
}

KeyValueStruct::KeyValueStruct(const String& key,
                               String     && val,
                               Format        format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(std::move(val)));
}

#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
  String KeyValueStruct::getUnit() const
  {
    return toUnitOfMeasureName(_uomIndex);
  }
#endif

void KeyValueStruct::setID(const String& id)                  { __id = id; }

void KeyValueStruct::setID(const __FlashStringHelper *id)     { __id = id; }

void KeyValueStruct::appendValue(ValueStruct&& value)
{
  _values.emplace_back(std::move(value));
  _isArray = true;
}

void KeyValueStruct::appendValue(const String& value)
{
  _values.emplace_back(ValueStruct(value));
  _isArray = true;
}

void KeyValueStruct::appendValue(const __FlashStringHelper * value)
{
  _values.emplace_back(ValueStruct(value));
  _isArray = true;
}

void KeyValueStruct::appendValue(String&& value)
{
  _values.emplace_back(ValueStruct(std::move(value)));
  _isArray = true;
}

String KeyValueStruct::getID() const
{
  if (__id.isEmpty()) { return to_internal_string(_key.toString(), '_'); }
  return __id.toString();
}
