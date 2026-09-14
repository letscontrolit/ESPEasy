#include "../DataStructs/ValueStruct.h"

#include "../Helpers/Numerical.h"
#include "../Helpers/Memory.h"
#include "../Helpers/PrintToString.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringConverter_Numerical.h"

#define VALUE_STRUCT_SSO_FIRST_CHAR_INDEX  2
#define VALUE_STRUCT_SSO_FIRST_CHAR   bytes_all[VALUE_STRUCT_SSO_FIRST_CHAR_INDEX]
#define VALUE_STRUCT_SSO_MAX_SIZE          13

const ValueStruct INVALID_VALUESTRUCT{};

// ********************************************************************************
// ValueStruct
// ********************************************************************************
ValueStruct::~ValueStruct()
{
  if (!_isSSO && (str_val != nullptr) &&
      ((static_cast<ValueStruct::ValueType>(_valueType) == ValueStruct::ValueType::String) ||
       (static_cast<ValueStruct::ValueType>(_valueType) == ValueStruct::ValueType::IP))
      ) {
    free(str_val);
  }
}

ValueStruct::ValueStruct(ValueStruct&& rhs)
{
  memcpy(bytes_all, rhs.bytes_all, sizeof(bytes_all));
  memset(rhs.bytes_all, 0, sizeof(bytes_all));
}

ValueStruct ValueStruct::makeHexFormatted(uint64_t val, uint8_t minNrDigits)
{
  ValueStruct res(val);

  res.setPreferredFormat(PreferredFormat::Hex);
  res.setMinNrDigits(minNrDigits);
  res.setCaseFormat(CaseFormat::ToUpper);
  return res;
}

ValueStruct ValueStruct::makeBinFormatted(uint64_t val, uint8_t minNrDigits)
{
  ValueStruct res(val);

  res.setPreferredFormat(PreferredFormat::Bin);
  res.setMinNrDigits(minNrDigits);
  return res;
}

void ValueStruct::setInt(const uint64_t& val) { this->operator=(ValueStruct(val)); }

void ValueStruct::setInt(const int64_t& val)  { this->operator=(ValueStruct(val)); }

void ValueStruct::setInt(int val)             { this->operator=(ValueStruct(val)); }

#if defined(ESP32) && !defined(__riscv)

void ValueStruct::setInt(int32_t val) { this->operator=(ValueStruct(val)); }

#endif // if defined(ESP32) && !defined(__riscv)

void ValueStruct::setInt(uint32_t val) { this->operator=(ValueStruct(val)); }

#if defined(ESP32) && !defined(__riscv)

void ValueStruct::setInt(size_t val) { this->operator=(ValueStruct(val)); }

#endif // if defined(ESP32) && !defined(__riscv)

void ValueStruct::setIPAddress(const IPAddress& ip, bool includeZone)
{
  #ifdef ESP8266
  this->operator=(ValueStruct(ip.toString()));
  #endif
  #ifdef ESP32
  this->operator=(ValueStruct(ip.toString(includeZone)));
  #endif
  _valueType = (uint64_t)ValueStruct::ValueType::IP;
}

void ValueStruct::setPreferredFormat(ValueStruct::PreferredFormat format)
{
  const auto vtype = getValueType();

  if ((vtype == ValueStruct::ValueType::Int) ||
      (vtype == ValueStruct::ValueType::UInt))
  {
    _preferredFormat = (uint64_t)format;
  }
}

void ValueStruct::setCaseFormat(ValueStruct::CaseFormat caseFormat) { _caseFormat = (uint64_t)caseFormat; }

void ValueStruct::clear() {
  if (!_isSSO
      && ((getValueType() == ValueStruct::ValueType::String) || (getValueType() == ValueStruct::ValueType::IP))
      && (str_val != nullptr)) {
    free(str_val);
  }
  memset(bytes_all, 0, sizeof(bytes_all));
}

ValueStruct& ValueStruct::operator=(ValueStruct&& rhs)
{
  clear();
  memcpy(bytes_all, rhs.bytes_all, sizeof(bytes_all));

  // Make sure rhs will not free allocated string
  memset(rhs.bytes_all, 0, sizeof(bytes_all));
  return *this;
}

ValueStruct& ValueStruct::deepCopy(const ValueStruct& rhs)
{
  if (!rhs._isSSO && (rhs.getValueType() == ValueStruct::ValueType::IP))
  {
    IPAddress ip;

    if (rhs.toIPAddress(ip)) {
      this->setIPAddress(ip);
    }
    return *this;
  }

  if (!rhs._isSSO && (rhs.getValueType() == ValueStruct::ValueType::String))
  {
    this->operator=(ValueStruct(rhs.toString()));
  } else {
    clear();

    // RHS does not have anything heap-allocated, so can make literal copy
    memcpy(bytes_all, rhs.bytes_all, sizeof(bytes_all));
  }
  return *this;
}

ValueStruct::ValueStruct(const bool& val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::Bool),
  _preferredFormat(0),
  _caseFormat(0),
  _size(1),
  _trimTrailingZeros(0),
  u64_val(val ? 1ull : 0ull)
{}

ValueStruct::ValueStruct(int val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::Int),
  _preferredFormat((uint64_t)PreferredFormat::Default),
  _caseFormat(0),
  _minNrDigits(1),
  _size(sizeof(val) * 8),
  i64_val(val)
{}

#if defined(ESP32) && !defined(__riscv)
ValueStruct::ValueStruct(int32_t val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::Int),
  _preferredFormat((uint64_t)PreferredFormat::Default),
  _caseFormat(0),
  _minNrDigits(1),
  _size(sizeof(val) * 8),
  i64_val(val)
{}

#endif // if defined(ESP32) && !defined(__riscv)
ValueStruct::ValueStruct(uint32_t val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::UInt),
  _preferredFormat((uint64_t)PreferredFormat::Default),
  _caseFormat(0),
  _minNrDigits(1),
  _size(sizeof(val) * 8),
  u64_val(val)
{}

#if defined(ESP32) && !defined(__riscv)
ValueStruct::ValueStruct(size_t val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::UInt),
  _preferredFormat((uint64_t)PreferredFormat::Default),
  _caseFormat(0),
  _minNrDigits(1),
  _size(sizeof(val) * 8),
  u64_val(val)
{}

#endif // if defined(ESP32) && !defined(__riscv)
ValueStruct::ValueStruct(const uint64_t& val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::UInt),
  _preferredFormat((uint64_t)PreferredFormat::Default),
  _caseFormat(0),
  _minNrDigits(1),
  _size(sizeof(val) * 8),
  u64_val(val)
{}

ValueStruct::ValueStruct(const int64_t& val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::Int),
  _preferredFormat((uint64_t)PreferredFormat::Default),
  _caseFormat(0),
  _minNrDigits(1),
  _size(sizeof(val) * 8),
  i64_val(val)
{}

ValueStruct::ValueStruct(const float& val,
                         uint8_t      nrDecimals,
                         bool         trimTrailingZeros) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::Float),
  _preferredFormat(0),
  _caseFormat(0),
  _nrDecimals((uint64_t)nrDecimals),
  _size(sizeof(val) * 8),
  _trimTrailingZeros((uint64_t)trimTrailingZeros),
  f_val(val)
{}

ValueStruct::ValueStruct(const double& val,
                         uint8_t       nrDecimals,
                         bool          trimTrailingZeros) :
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::Double),
  _preferredFormat(0),
  _caseFormat(0),
  _nrDecimals((uint64_t)nrDecimals),
  _size(sizeof(val) * 8),
  _trimTrailingZeros((uint64_t)trimTrailingZeros),
  d_val(val)
#else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::Float),
  _preferredFormat(0),
  _caseFormat(0),
  _nrDecimals((uint64_t)nrDecimals),
  _size(sizeof(f_val) * 8),
  _trimTrailingZeros((uint64_t)trimTrailingZeros),
  f_val(val)
#endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
{}

ValueStruct::ValueStruct(const char*val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::String),
  _preferredFormat(0),
  _caseFormat(0),
  _size(val ? strlen_P((const char *)(val)) : 0),
  str_val(nullptr)
{
  if (_size <= VALUE_STRUCT_SSO_MAX_SIZE) {
    _isSSO = true;

    // Make sure it is zero-terminated when nullptr is given
    VALUE_STRUCT_SSO_FIRST_CHAR = 0;

    if (_size > 0) {
      memcpy(&VALUE_STRUCT_SSO_FIRST_CHAR, (uint8_t *)val, _size + 1);
    }
  } else {
    str_val = special_calloc(1, _size + 1);

    if (str_val) {
      memcpy_P(str_val, val, _size);
    }
  }
}

ValueStruct::ValueStruct(const String& val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::String),
  _preferredFormat(0),
  _caseFormat(0),
  _size(val.length()),
  str_val(nullptr)
{
  if (_size <= VALUE_STRUCT_SSO_MAX_SIZE) {
    _isSSO = true;

    if (_size == 0) {
      // Make sure it is zero-terminated
      VALUE_STRUCT_SSO_FIRST_CHAR = 0;
    }
    else {
      memcpy(&VALUE_STRUCT_SSO_FIRST_CHAR, (uint8_t *)val.c_str(), val.length() + 1);
    }
  } else {
    str_val = special_calloc(1, _size + 1);

    if (str_val) {
      memcpy_P(str_val, (uint8_t *)val.c_str(), val.length() + 1);
    }
  }
}

ValueStruct::ValueStruct(String&& val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::String),
  _preferredFormat(0),
  _caseFormat(0),
  _size(val.length()),
  str_val(nullptr)
{
  if (_size <= VALUE_STRUCT_SSO_MAX_SIZE) {
    _isSSO = true;

    if (_size == 0) {
      // Make sure it is zero-terminated
      VALUE_STRUCT_SSO_FIRST_CHAR = 0;
    }
    else {
      memcpy(&VALUE_STRUCT_SSO_FIRST_CHAR, (uint8_t *)val.c_str(), val.length() + 1);
    }
  } else {
    str_val = special_calloc(1, _size + 1);

    if (str_val) {
      memcpy_P(str_val, (uint8_t *)val.c_str(), val.length() + 1);
    }
  }

  // We can't move the allocated memory from 'message'.
  // Just use move so we make sure the memory is de-allocated after this call.
  const String str = std::move(val);
}

ValueStruct::ValueStruct(const __FlashStringHelper *val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::FlashString),
  _preferredFormat(0),
  _caseFormat(0),
  _size(val ? strlen_P((const char *)(val)) : 0),
  str_val((void *)(val))
{}

ValueStruct ValueStruct::makeFromString(const __FlashStringHelper *val)
{
  auto res(makeFromString(String(val)));

  if (res.getValueType() == ValueStruct::ValueType::String)
  {
    // We can store it as a flash string, which doesn't need memory allocation
    return ValueStruct(val);
  }
  return res;
}

ValueStruct ValueStruct::makeFromString(const String& val)
{
  String numStr;
  NumericalType detectedType = NumericalType::Not_a_number;
  bool negativeValue{};

  if (!val.isEmpty())
  {
    String trimmedVal(val);
    trimmedVal.trim();

    if (trimmedVal.isEmpty()) {
      return ValueStruct(val);
    }

    const bool b_true = trimmedVal.equalsIgnoreCase(F("true"));

    if (b_true || trimmedVal.equalsIgnoreCase(F("false"))) {
      return ValueStruct(b_true);
    }

    {
      // Check for IP before checking for other types,
      // as it would otherwise be stored as a string, not IP type.
      IPAddress ip;

      if (ip.fromString(val))
      {
        ValueStruct v;
        v.setIPAddress(ip);
        return v;
      }
    }

    if (ContainsAny(val, F(":#%[](){},\"'`"))) {
      return ValueStruct(val);
    }

    numStr = getNumerical(trimmedVal, NumericalType::FloatingPoint, detectedType);
    numStr.trim();

    if (!numStr.isEmpty() && (numStr[0] == '-')) {
      negativeValue = true;
    }
  }

  switch (detectedType)
  {
    case NumericalType::Not_a_number:
      break;
    case NumericalType::FloatingPoint:
    {
      ESPEASY_RULES_FLOAT_TYPE val_f{};
      int nrDec{};

      if (validDoubleFromString(numStr, val_f, nrDec) && (nrDec >= 0))
      {
        return ValueStruct(val_f, nrDec);
      }
      break;
    }
    default:
    {
      ValueStruct res;

      if (negativeValue) {
        int64_t result{};

        if (validInt64FromString(numStr, result))
        {
          if (std::numeric_limits<int32_t>::min() < result) {
            res = ValueStruct(static_cast<int32_t>(result));
          }
          else {
            res = ValueStruct(result);
          }
        }
      } else {
        uint64_t result{};

        if (validUInt64FromString(numStr, result))
        {
          if (std::numeric_limits<uint32_t>::max() > result) {
            res = ValueStruct(static_cast<uint32_t>(result));
          }
          else {
            res = ValueStruct(result);
          }
        }
      }

      if (res.isSet()) {
        PreferredFormat format(PreferredFormat::Default);

        if (detectedType == NumericalType::BinaryUint) { format = PreferredFormat::Bin; }
        else if (detectedType == NumericalType::HexadecimalUInt) { format = PreferredFormat::Hex; }

        res.setPreferredFormat(format);
        return res;
      }
      break;
    }
  }

  // Just store it as a string type
  return ValueStruct(val);
}

String ValueStruct::toString(bool unformatted) const
{
  ValueType valueType;

  return toString(valueType, unformatted);
}

String ValueStruct::toString(ValueType& valueType, bool unformatted) const
{
  PrintToString p;

  print(p, valueType, unformatted);
  String res(p.getMove());
  return res;
}

int64_t ValueStruct::toInt(int64_t defaultValue) const
{
  switch (getValueType())
  {
    case ValueStruct::ValueType::Bool:
    {
      return u64_val == 0 ? 0 : 1;
    }
    case ValueStruct::ValueType::Int:
    {
      return i64_val;
    }
    case ValueStruct::ValueType::UInt:
    {
      if (u64_val < std::numeric_limits<int64_t>::max()) {
        return u64_val;
      }
      break;
    }
    case ValueStruct::ValueType::Float:
#if !FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
    case ValueStruct::ValueType::Double:
#endif

      if (isValidFloat(f_val)) {
        return roundf(f_val);
      }
      break;
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
    case ValueStruct::ValueType::Double:

      if (isValidDouble(d_val)) {
        return round(d_val);
      }
      break;
#endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
    case ValueStruct::ValueType::String:
    case ValueStruct::ValueType::FlashString:

      if (_size) {
        return toString().toInt();
      }
      break;
    case ValueStruct::ValueType::Unset:
    case ValueStruct::ValueType::IP:
      break;
  }
  return defaultValue;
}

ESPEASY_RULES_FLOAT_TYPE ValueStruct::toFloat(ESPEASY_RULES_FLOAT_TYPE defaultValue) const
{
  switch (getValueType())
  {
    case ValueStruct::ValueType::Bool:
    {
      return u64_val == 0 ? 0 : 1;
    }
    case ValueStruct::ValueType::Int:
    {
      return i64_val;
    }
    case ValueStruct::ValueType::UInt:
    {
      return u64_val;
    }
    case ValueStruct::ValueType::Float:
    {
      return f_val;
    }
    case ValueStruct::ValueType::Double:
    {
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      return d_val;
#else
      return f_val;
#endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
    }
    case ValueStruct::ValueType::String:
    case ValueStruct::ValueType::FlashString:
    case ValueStruct::ValueType::IP:
    case ValueStruct::ValueType::Unset:
      break;
  }
  return defaultValue;
}

bool ValueStruct::toIPAddress(IPAddress& ip) const
{
  switch (getValueType())
  {
    case ValueStruct::ValueType::IP:
    case ValueStruct::ValueType::String:
    case ValueStruct::ValueType::FlashString:
      return ip.fromString(this->toString());
    default:
      break;
  }
  return false;
}

size_t ValueStruct::print(Print& out) const
{
  ValueStruct::ValueType v;

  return print(out, v);
}

size_t ValueStruct::formatCase(Print& out, String&& str) const
{
  switch (getCaseFormat())
  {
    case CaseFormat::ToLower:
    {
      str.toLowerCase();
      break;
    }
    case CaseFormat::ToUpper:
    {
      str.toUpperCase();
      break;
    }
    default:
      break;
  }
  return out.print(str);
}

size_t ValueStruct::print(Print& out, ValueType& valueType, bool unformatted) const
{
  valueType = getValueType();

  if (_isSSO) {
    if (getCaseFormat(unformatted) == CaseFormat::KeepCase) {
      return out.write((const char *)&VALUE_STRUCT_SSO_FIRST_CHAR);
    }
    return formatCase(out, String((const char *)&VALUE_STRUCT_SSO_FIRST_CHAR));
  }

  switch (valueType)
  {
    case ValueStruct::ValueType::Bool:
    {
      return out.print(u64_val == 0 ? 0 : 1);
    }
    case ValueStruct::ValueType::String:
    {
      if (str_val == nullptr) { return 0; }

      if (getCaseFormat(unformatted) == CaseFormat::KeepCase) {
        return out.write((const uint8_t *)str_val, _size);
      }
      return formatCase(out, String((const char *)str_val));
    }
    case ValueStruct::ValueType::FlashString:
    {
      if (str_val == nullptr) { return 0; }

      if (getCaseFormat(unformatted) == CaseFormat::KeepCase) {
        return out.print((const __FlashStringHelper *)str_val);
      }
      return formatCase(out, String((const __FlashStringHelper *)str_val));
    }
    case ValueStruct::ValueType::IP:
    {
      return out.write((const uint8_t *)str_val, _size);
    }
    case ValueStruct::ValueType::Float:
#if !FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
    case ValueStruct::ValueType::Double:
#endif
    {
      String res;

      if (!toValidString(res, f_val, _nrDecimals, _trimTrailingZeros))
      {
        valueType = ValueStruct::ValueType::String;
      }
      return out.print(res);
    }
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
    case ValueStruct::ValueType::Double:
    {
      String res;

      if (!doubleToValidString(res, d_val, _nrDecimals, _trimTrailingZeros))
      {
        valueType = ValueStruct::ValueType::String;
      }
      return out.print(res);
    }
#endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
    case ValueStruct::ValueType::Int:
    {
      if ((_size > 32) || _minNrDigits) {
        return out.print(ll2String(i64_val, DEC, _minNrDigits, '\0', getCaseFormat(unformatted) == CaseFormat::ToUpper));
      }
      auto v = static_cast<int32_t>(i64_val);
      return out.print(v);
    }
    case ValueStruct::ValueType::UInt:
    {
      const auto format = getPreferredFormat(unformatted);

      if (format == PreferredFormat::Bin) {
        return out.print(concat(F("0b"), ull2String(u64_val, BIN, _minNrDigits)));
      }

      if (format == PreferredFormat::Hex) {
        return out.print(concat(F("0x"), ull2String(u64_val, HEX, _minNrDigits, '\0', getCaseFormat(unformatted) == CaseFormat::ToUpper)));
      }

      if ((_size > 32) || _minNrDigits) {
        return out.print(ull2String(u64_val, DEC, _minNrDigits));
      }
      auto v = static_cast<uint32_t>(u64_val);
      return out.print(v);
    }
    case ValueStruct::ValueType::Unset:
      break;
  }
  return 0;
}

bool ValueStruct::isEmpty() const
{
  if (_isSSO) { return VALUE_STRUCT_SSO_FIRST_CHAR == 0; }

  switch (getValueType())
  {
    case ValueStruct::ValueType::String:
    case ValueStruct::ValueType::FlashString:
    case ValueStruct::ValueType::IP:
      return _size == 0 || str_val == nullptr;
    case ValueStruct::ValueType::Unset:
      return true;
    default: break;
  }
  return false;
}

bool ValueStruct::equals(const __FlashStringHelper *cmdStr, bool ignoreCase) const
{
  if (getValueType() == ValueStruct::ValueType::Unset) { return false; }
  return equals(String(cmdStr), ignoreCase);
}

bool ValueStruct::equals(const String& cmdStr, bool ignoreCase) const
{
  if (getValueType() == ValueStruct::ValueType::Unset) { return false; }

  if (ignoreCase) {
    return toString().equalsIgnoreCase(cmdStr);
  }

  return toString().equals(cmdStr);
}

#ifndef BUILD_NO_DEBUG

String ValueStruct::debug() const
{
  const String normalStr      = toString();
  const String unformattedStr = toString(true);
  String logstr               = strformat(
    F("%s:'%s'"),
    toShortStr(getValueType()),
    normalStr.c_str());

  if (!normalStr.equals(unformattedStr)) {
    logstr += strformat(F("/u:'%s'"), unformattedStr.c_str());
  }

  if (!operator bool()) {
    logstr += '!';
  }

  return logstr;
}

#endif // ifndef BUILD_NO_DEBUG
