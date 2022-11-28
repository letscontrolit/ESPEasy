// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2022, Benoit BLANCHON
// MIT License

#pragma once

#include <stddef.h>  // size_t
#include <string.h>  // strcmp

#include <ArduinoJson/Polyfills/assert.hpp>
#include <ArduinoJson/Strings/StoragePolicy.hpp>
#include <ArduinoJson/Strings/StringAdapter.hpp>

namespace ARDUINOJSON_NAMESPACE {

class ZeroTerminatedRamString {
 public:
  static const size_t typeSortKey = 3;

  ZeroTerminatedRamString(const char* str) : _str(str) {}

  bool isNull() const {
    return !_str;
  }

  size_t size() const {
    return _str ? ::strlen(_str) : 0;
  }

  char operator[](size_t i) const {
    ARDUINOJSON_ASSERT(_str != 0);
    ARDUINOJSON_ASSERT(i <= size());
    return _str[i];
  }

  const char* data() const {
    return _str;
  }

  friend int stringCompare(ZeroTerminatedRamString a,
                           ZeroTerminatedRamString b) {
    ARDUINOJSON_ASSERT(!a.isNull());
    ARDUINOJSON_ASSERT(!b.isNull());
    return ::strcmp(a._str, b._str);
  }

  friend bool stringEquals(ZeroTerminatedRamString a,
                           ZeroTerminatedRamString b) {
    return stringCompare(a, b) == 0;
  }

  StringStoragePolicy::Copy storagePolicy() {
    return StringStoragePolicy::Copy();
  }

 protected:
  const char* _str;
};

template <typename TChar>
struct StringAdapter<TChar*, typename enable_if<sizeof(TChar) == 1>::type> {
  typedef ZeroTerminatedRamString AdaptedString;

  static AdaptedString adapt(const TChar* p) {
    return AdaptedString(reinterpret_cast<const char*>(p));
  }
};

template <typename TChar, size_t N>
struct StringAdapter<TChar[N], typename enable_if<sizeof(TChar) == 1>::type> {
  typedef ZeroTerminatedRamString AdaptedString;

  static AdaptedString adapt(const TChar* p) {
    return AdaptedString(reinterpret_cast<const char*>(p));
  }
};

class StaticStringAdapter : public ZeroTerminatedRamString {
 public:
  StaticStringAdapter(const char* str) : ZeroTerminatedRamString(str) {}

  StringStoragePolicy::Link storagePolicy() {
    return StringStoragePolicy::Link();
  }
};

template <>
struct StringAdapter<const char*, void> {
  typedef StaticStringAdapter AdaptedString;

  static AdaptedString adapt(const char* p) {
    return AdaptedString(p);
  }
};

class SizedRamString {
 public:
  static const size_t typeSortKey = 2;

  SizedRamString(const char* str, size_t sz) : _str(str), _size(sz) {}

  bool isNull() const {
    return !_str;
  }

  size_t size() const {
    return _size;
  }

  char operator[](size_t i) const {
    ARDUINOJSON_ASSERT(_str != 0);
    ARDUINOJSON_ASSERT(i <= size());
    return _str[i];
  }

  const char* data() const {
    return _str;
  }

  StringStoragePolicy::Copy storagePolicy() {
    return StringStoragePolicy::Copy();
  }

 protected:
  const char* _str;
  size_t _size;
};

template <typename TChar>
struct SizedStringAdapter<TChar*,
                          typename enable_if<sizeof(TChar) == 1>::type> {
  typedef SizedRamString AdaptedString;

  static AdaptedString adapt(const TChar* p, size_t n) {
    return AdaptedString(reinterpret_cast<const char*>(p), n);
  }
};

}  // namespace ARDUINOJSON_NAMESPACE
