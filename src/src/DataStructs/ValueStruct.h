#pragma once

#include <WString.h>
#include <Print.h>

// ********************************************************************************
// ValueStruct
// ********************************************************************************


class ValueStruct
{
public:

  enum class ValueType : uint8_t {
    Unset = 0,
    String,
    FlashString,
    Float,
    Double,
    Int,
    UInt,
    Bool

  };

  ValueStruct() :
    _isSSO(0),
    _valueType((uint64_t)ValueStruct::ValueType::Unset),
    str_val(nullptr)
  {}

  ~ValueStruct();

  ValueStruct(const ValueStruct& rhs) = delete;
  ValueStruct(ValueStruct&& rhs);

  ValueStruct(const bool& val);

  ValueStruct(int val);
#if defined(ESP32) && !defined(__riscv)
  ValueStruct(int32_t val);
#endif
  ValueStruct(uint32_t val);
#if defined(ESP32) && !defined(__riscv)
  ValueStruct(size_t val);
#endif
  ValueStruct(const uint64_t& val);

  ValueStruct(const int64_t& val);


  ValueStruct(const float& val,
              uint8_t      nrDecimals        = 4,
              bool         trimTrailingZeros = false);

  ValueStruct(const double& val,
              uint8_t       nrDecimals        = 4,
              bool          trimTrailingZeros = false);


  ValueStruct(const char*val);

  ValueStruct(const String& val);

  ValueStruct(String&& val);

  ValueStruct(const __FlashStringHelper *val);

  ValueStruct::ValueType getValueType() const
  {
    if (_isSSO) { return ValueStruct::ValueType::String; }
    return static_cast<ValueStruct::ValueType>(_valueType);
  }

  ValueStruct& operator=(ValueStruct&& rhs);
  ValueStruct& operator=(const ValueStruct& rhs) = delete;

  String       toString() const;

  String       toString(ValueType& valueType) const;

  int64_t      toInt() const;

  double       toFloat() const;

  size_t       print(Print& out) const;

  bool         isEmpty() const;

  bool         isSet() const { return getValueType() != ValueStruct::ValueType::Unset; }

private:

  size_t print(Print    & out,
               ValueType& valueType) const;

  union {
    struct {
      uint64_t _isSSO             : 1;
      uint64_t _trimTrailingZeros : 1;
      uint64_t _valueType         : 6;
      uint64_t _nrDecimals        : 8;
      uint64_t _size              : 16;

      uint64_t unused : 32;

      union {
        void    *str_val;
        float    f_val;
        double   d_val;
        int64_t  i64_val;
        uint64_t u64_val;

      };

    };

    // When _isSSO, the short string will be stored in bytes 1 ... 15
    // The short string will be zero-terminated.
    uint8_t bytes_all[16] = {};

  };


}; // class ValueStruct
