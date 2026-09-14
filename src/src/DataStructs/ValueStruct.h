#pragma once

#include <WString.h>
#include <Print.h>

#include <IPAddress.h>

// ********************************************************************************
// ValueStruct
// ********************************************************************************


class ValueStruct
{
public:

  // Don't just extend these enums as they are part of very tightly packed struct.
  // For example if 1 extra bit is needed, the start and length of the SSO must be adjusted.

  enum class ValueType : uint8_t {
    Unset = 0,
    String,
    FlashString,
    IP,
    Float,
    Double,
    Int,
    UInt,
    Bool

  };

  #ifndef BUILD_NO_DEBUG

  static const char* toShortStr(ValueType valueType)
  {
    static const char*shortStrings = "---\0Str\0Fla\0IP\0\0flt\0dbl\0int\0uin\0boo\0";
    constexpr uint8_t index_modulo = static_cast<uint8_t>(ValueType::Bool) + 1;
    const uint8_t     index        = static_cast<uint8_t>(valueType) % index_modulo;

    return shortStrings + 4 * index;
  }

  #endif // ifndef BUILD_NO_DEBUG

  enum class PreferredFormat : uint8_t {
    Default = 0,
    Bin,
    Hex

  };

  enum class CaseFormat : uint8_t {
    KeepCase = 0,
    ToUpper,
    ToLower

  };

  ValueStruct() :
    _isSSO(0),
    _valueType((uint64_t)ValueStruct::ValueType::Unset),
    str_val(nullptr)
  {
    memset(bytes_all, 0, sizeof(bytes_all));
  }

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

  static ValueStruct     makeHexFormatted(uint64_t val,
                                          uint8_t  minNrDigits);
  static ValueStruct     makeBinFormatted(uint64_t val,
                                          uint8_t  minNrDigits);

  void                   setInt(const uint64_t& val);
  void                   setInt(const int64_t& val);

  void                   setInt(int val);
#if defined(ESP32) && !defined(__riscv)
  void                   setInt(int32_t val);
#endif
  void                   setInt(uint32_t val);
#if defined(ESP32) && !defined(__riscv)
  void                   setInt(size_t val);
#endif

  void                   setIPAddress(const IPAddress& ip, bool includeZone = false);

  ValueStruct::ValueType getValueType() const
  {
    if (_isSSO) { return ValueStruct::ValueType::String; }
    return static_cast<ValueStruct::ValueType>(_valueType);
  }

  ValueStruct::PreferredFormat getPreferredFormat(bool unformatted = false) const
  {
    if (unformatted) { return ValueStruct::PreferredFormat::Default; }
    return static_cast<ValueStruct::PreferredFormat>(_preferredFormat);
  }

  void                    setPreferredFormat(ValueStruct::PreferredFormat format);

  ValueStruct::CaseFormat getCaseFormat(bool unformatted = false) const {
    if (unformatted) { return ValueStruct::CaseFormat::KeepCase; }
    return static_cast<ValueStruct::CaseFormat>(_caseFormat);
  }

  void         setCaseFormat(ValueStruct::CaseFormat caseFormat);

  void         setMinNrDigits(uint8_t minNrDigits) { _minNrDigits = (uint64_t)minNrDigits; }

  void         clear();

  ValueStruct& operator=(ValueStruct&& rhs);
  ValueStruct& operator=(const ValueStruct& rhs) = delete;

  // We really try to enforce moving the ValueStruct, but when needed a deepcopy is possible
  ValueStruct& deepCopy(const ValueStruct& rhs);

  operator bool() const {
    return getValueType() != ValueStruct::ValueType::Unset;
  }

  // Try to interpret the given string and store the value as compact as possible
  // If the given string is a numerical, try to detect:
  //  - preferred notation (Hex/Dec/Bin)
  //  - number of decimals
  static ValueStruct       makeFromString(const __FlashStringHelper *val);
  static ValueStruct       makeFromString(const String& val);

  String                   toString(bool unformatted = false) const;

  String                   toString(ValueType& valueType,
                                    bool       unformatted = false) const;

  int64_t                  toInt(int64_t defaultValue = 0) const;

  ESPEASY_RULES_FLOAT_TYPE toFloat(ESPEASY_RULES_FLOAT_TYPE defaultValue = 0) const;

  bool                     toIPAddress(IPAddress& ip) const;

  size_t                   print(Print& out) const;

  bool                     isEmpty() const;

  bool                     isSet() const { return getValueType() != ValueStruct::ValueType::Unset; }

  bool                     equals(const __FlashStringHelper *cmdStr,
                                  bool                       ignoreCase = false) const;
  bool                     equals(const String& cmdStr,
                                  bool          ignoreCase = false) const;

  bool                     equalsIgnoreCase(const __FlashStringHelper *cmdStr) const { return equals(cmdStr, true); }

  bool                     equalsIgnoreCase(const String& cmdStr) const              { return equals(cmdStr, true); }

#ifndef BUILD_NO_DEBUG
  String                   debug() const;
#endif

private:

  size_t formatCase(Print  & out,
                    String&& str) const;

  size_t print(Print    & out,
               ValueType& valueType,
               bool       unformatted = false) const;

  union {
    struct {
      // Need to have the most important bits for formatting strings in the first byte
      // When more bits are needed for storing SSO strings,
      // move the first byte of SSO string (VALUE_STRUCT_SSO_FIRST_CHAR_INDEX)
      // and decrease max. nr of SSO bytes (VALUE_STRUCT_SSO_MAX_SIZE)

      uint64_t _isSSO           : 1;
      uint64_t _valueType       : 4;
      uint64_t _preferredFormat : 2;
      uint64_t _caseFormat      : 2;
      uint64_t _minNrDigits     : 7; // For printing ints with leading zeroes

      // --- End of 2nd byte

      uint64_t _nrDecimals        : 8;
      uint64_t _size              : 16; // Length of string or nr of bits
      uint64_t _trimTrailingZeros : 1;
      uint64_t unused             : 23;

      union {
        void *str_val;
        float f_val;
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
        double d_val;
#endif
        int64_t  i64_val;
        uint64_t u64_val;

      };

    };

    // When _isSSO, the short string will be stored in bytes 1 ... 15
    // The short string will be zero-terminated.
    uint8_t bytes_all[16] = {};

  };


}; // class ValueStruct

extern const ValueStruct INVALID_VALUESTRUCT;
