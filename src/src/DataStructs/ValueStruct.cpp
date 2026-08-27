#include "../DataStructs/ValueStruct.h"

#include "../Helpers/Memory.h"
#include "../Helpers/PrintToString.h"
#include "../Helpers/StringConverter_Numerical.h"

#define VALUE_STRUCT_SSO_FIRST_CHAR_INDEX  1
#define VALUE_STRUCT_SSO_FIRST_CHAR   bytes_all[VALUE_STRUCT_SSO_FIRST_CHAR_INDEX]
#define VALUE_STRUCT_SSO_MAX_SIZE          14

// ********************************************************************************
// ValueStruct
// ********************************************************************************
ValueStruct::~ValueStruct()
{
  const auto vtype = static_cast<ValueStruct::ValueType>(_valueType);

  if (!_isSSO && (vtype == ValueStruct::ValueType::String))
  {
    if (str_val != nullptr) {
      free(str_val);
    }
  }
}

ValueStruct::ValueStruct(ValueStruct&& rhs)
{
  memcpy(bytes_all, rhs.bytes_all, sizeof(bytes_all));
  memset(rhs.bytes_all, 0, sizeof(bytes_all));
}

ValueStruct& ValueStruct::operator=(ValueStruct&& rhs)
{
  memcpy(bytes_all, rhs.bytes_all, sizeof(bytes_all));
  memset(rhs.bytes_all, 0, sizeof(bytes_all));
  return *this;
}

ValueStruct::ValueStruct(const bool& val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::Bool),
  _size(1),
  u64_val(val ? 1ull : 0ull)
{}

ValueStruct::ValueStruct(int val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::Int),
  _size(sizeof(val) * 8),
  i64_val(val)
{}

#if defined(ESP32) && !defined(__riscv)
ValueStruct::ValueStruct(int32_t val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::Int),
  _size(sizeof(val) * 8),
  i64_val(val)
{}

#endif // if defined(ESP32) && !defined(__riscv)
ValueStruct::ValueStruct(uint32_t val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::UInt),
  _size(sizeof(val) * 8),
  u64_val(val)
{}

#if defined(ESP32) && !defined(__riscv)
ValueStruct::ValueStruct(size_t val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::UInt),
  _size(sizeof(val) * 8),
  u64_val(val)
{}

#endif // if defined(ESP32) && !defined(__riscv)
ValueStruct::ValueStruct(const uint64_t& val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::UInt),
  _size(sizeof(val) * 8),
  u64_val(val)
{}

ValueStruct::ValueStruct(const int64_t& val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::Int),
  _size(sizeof(val) * 8),
  i64_val(val)
{}

ValueStruct::ValueStruct(const float& val,
                         uint8_t      nrDecimals,
                         bool         trimTrailingZeros) :
  _isSSO(0),
  _trimTrailingZeros((uint64_t)trimTrailingZeros),
  _valueType((uint64_t)ValueStruct::ValueType::Float),
  _nrDecimals((uint64_t)nrDecimals),
  _size(sizeof(val) * 8),
  f_val(val)
{}

ValueStruct::ValueStruct(const double& val,
                         uint8_t       nrDecimals,
                         bool          trimTrailingZeros) :
  _isSSO(0),
  _trimTrailingZeros((uint64_t)trimTrailingZeros),
  _valueType((uint64_t)ValueStruct::ValueType::Double),
  _nrDecimals((uint64_t)nrDecimals),
  _size(sizeof(val) * 8),
  d_val(val)
{}

ValueStruct::ValueStruct(const char*val) :
  _isSSO(0),
  _valueType((uint64_t)ValueStruct::ValueType::String),
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
  _size(val ? strlen_P((const char *)(val)) : 0),
  str_val((void *)(val))
{}

String ValueStruct::toString() const
{
  ValueType valueType;

  return toString(valueType);
}

String ValueStruct::toString(ValueType& valueType) const
{
  PrintToString p;

  print(p, valueType);
  String res(p.getMove());
  return res;
}

int64_t ValueStruct::toInt() const
{
  switch(getValueType())
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
    case ValueStruct::ValueType::String:
    case ValueStruct::ValueType::FlashString:
    case ValueStruct::ValueType::Float:
    case ValueStruct::ValueType::Double:
    case ValueStruct::ValueType::Unset:
      break;
  }
  return toString().toInt();
}

double ValueStruct::toFloat() const
{
  switch(getValueType())
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
      return d_val;
    }
    case ValueStruct::ValueType::String:
    case ValueStruct::ValueType::FlashString:
    case ValueStruct::ValueType::Unset:
      break;
  }
  return 0.0;
}

size_t ValueStruct::print(Print& out) const
{
  ValueStruct::ValueType v;

  return print(out, v);
}

size_t ValueStruct::print(Print& out, ValueType& valueType) const
{
  valueType = getValueType();

  if (_isSSO) {
    return out.write((const char *)&VALUE_STRUCT_SSO_FIRST_CHAR);
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
      return out.write((const uint8_t *)str_val, _size);
    }
    case ValueStruct::ValueType::FlashString:
    {
      if (str_val == nullptr) { return 0; }
      return out.print((const __FlashStringHelper *)str_val);
    }
    case ValueStruct::ValueType::Float:
    {
      String res;

      if (!toValidString(res, f_val, _nrDecimals, _trimTrailingZeros))
      {
        valueType = ValueStruct::ValueType::String;
      }
      return out.print(res);
    }
    case ValueStruct::ValueType::Double:
    {
      String res;

      if (!doubleToValidString(res, d_val, _nrDecimals, _trimTrailingZeros))
      {
        valueType = ValueStruct::ValueType::String;
      }
      return out.print(res);
    }
    case ValueStruct::ValueType::Int:
    {
      if (_size == 8) {
        return out.print(ll2String(i64_val));
      }
      auto v = static_cast<int32_t>(i64_val);
      return out.print(v);
    }
    case ValueStruct::ValueType::UInt:
    {
      if (_size == 8) {
        return out.print(ull2String(u64_val));
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
      return _size == 0;
    case ValueStruct::ValueType::Unset:
      return true;
    default: break;
  }
  return false;
}
