#include "../DataStructs/KeyValueStruct.h"

#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
# include "../Helpers/ESPEasy_UnitOfMeasure.h"
#endif
#include "../Helpers/StringConverter.h"

// ********************************************************************************
// KeyValueStruct
// ********************************************************************************

KeyValueStruct::KeyValueStruct(const __FlashStringHelper *key, Format format) : _key(key), _format(format) {}

KeyValueStruct::KeyValueStruct(const String& key, Format format) : _key(key), _format(format) {}

KeyValueStruct::KeyValueStruct(const __FlashStringHelper *key,
                               const bool               & val,
                               Format                     format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

KeyValueStruct::KeyValueStruct(const String& key,
                               const bool  & val,
                               Format        format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

KeyValueStruct::KeyValueStruct(const __FlashStringHelper *key,
                               int                        val,
                               Format                     format)
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
KeyValueStruct::KeyValueStruct(const __FlashStringHelper *key,
                               int32_t                    val,
                               Format                     format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

KeyValueStruct::KeyValueStruct(const String& key,
                               int32_t       val,
                               Format        format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

#endif // if defined(ESP32) && !defined(__riscv)

KeyValueStruct::KeyValueStruct(const __FlashStringHelper *key,
                               uint32_t                   val,
                               Format                     format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

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
KeyValueStruct::KeyValueStruct(const __FlashStringHelper *key,
                               const uint64_t           & val,
                               Format                     format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

KeyValueStruct::KeyValueStruct(const String  & key,
                               const uint64_t& val,
                               Format          format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

KeyValueStruct::KeyValueStruct(const __FlashStringHelper *key,
                               const int64_t            & val,
                               Format                     format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

KeyValueStruct::KeyValueStruct(const String & key,
                               const int64_t& val,
                               Format         format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val));
}

KeyValueStruct::KeyValueStruct(const __FlashStringHelper *key,
                               const float              & val,
                               uint8_t                    nrDecimals,
                               Format                     format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val, nrDecimals));
}

KeyValueStruct::KeyValueStruct(const String& key,
                               const float & val,
                               uint8_t       nrDecimals,
                               Format        format)
  : _key(key), _format(format) {
  _values.emplace_back(ValueStruct(val, nrDecimals));
}

KeyValueStruct::KeyValueStruct(const __FlashStringHelper *key,
                               const double             & val,
                               uint8_t                    nrDecimals,
                               Format                     format)
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

void KeyValueStruct::clear()
{
  _key.clear();
  _values.clear();
  _format = Format::Default;
#if FEATURE_TASKVALUE_UNIT_OF_MEASURE
  _uomIndex = 0;
#endif
  _isArray = false;
}

KeyValueStruct KeyValueStruct::makeHexFormatted(const __FlashStringHelper *key, uint64_t val, uint8_t minNrDigits)
{
  KeyValueStruct kv(
    key,
    KeyValueStruct::Format::PreFormatted);

  kv.setValue(ValueStruct::makeHexFormatted(val, minNrDigits));
  return kv;
}

#if FEATURE_TASKVALUE_UNIT_OF_MEASURE

String KeyValueStruct::getUnit() const
{
  return toUnitOfMeasureName(_uomIndex);
}

#endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE

void KeyValueStruct::setID(const String& id)              { __id = id; }

void KeyValueStruct::setID(const __FlashStringHelper *id) { __id = id; }

void KeyValueStruct::setValue(ValueStruct&& value)
{
  _values.clear();
  _isArray = false;
  _values.emplace_back(std::move(value));
}

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

void KeyValueStruct::appendValue(const __FlashStringHelper *value)
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
