// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2023, Benoit BLANCHON
// MIT License

#pragma once

#include <ArduinoJson/Configuration.hpp>
#include <ArduinoJson/Numbers/arithmeticCompare.hpp>
#include <ArduinoJson/Polyfills/type_traits.hpp>
#include <ArduinoJson/Strings/StringAdapters.hpp>
#include <ArduinoJson/Variant/Visitor.hpp>

ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE

class CollectionData;

struct ComparerBase : Visitor<CompareResult> {};

template <typename T, typename Enable = void>
struct Comparer;

template <typename T>
struct Comparer<T, typename enable_if<IsString<T>::value>::type>
    : ComparerBase {
  T rhs;  // TODO: store adapted string?

  explicit Comparer(T value) : rhs(value) {}

  CompareResult visitString(const char* lhs, size_t n) {
    int i = stringCompare(adaptString(rhs), adaptString(lhs, n));
    if (i < 0)
      return COMPARE_RESULT_GREATER;
    else if (i > 0)
      return COMPARE_RESULT_LESS;
    else
      return COMPARE_RESULT_EQUAL;
  }

  CompareResult visitNull() {
    if (adaptString(rhs).isNull())
      return COMPARE_RESULT_EQUAL;
    else
      return COMPARE_RESULT_DIFFER;
  }
};

template <typename T>
struct Comparer<T, typename enable_if<is_integral<T>::value ||
                                      is_floating_point<T>::value>::type>
    : ComparerBase {
  T rhs;

  explicit Comparer(T value) : rhs(value) {}

  CompareResult visitFloat(JsonFloat lhs) {
    return arithmeticCompare(lhs, rhs);
  }

  CompareResult visitSignedInteger(JsonInteger lhs) {
    return arithmeticCompare(lhs, rhs);
  }

  CompareResult visitUnsignedInteger(JsonUInt lhs) {
    return arithmeticCompare(lhs, rhs);
  }

  CompareResult visitBoolean(bool lhs) {
    return visitUnsignedInteger(static_cast<JsonUInt>(lhs));
  }
};

struct NullComparer : ComparerBase {
  CompareResult visitNull() {
    return COMPARE_RESULT_EQUAL;
  }
};

template <>
struct Comparer<decltype(nullptr), void> : NullComparer {
  explicit Comparer(decltype(nullptr)) : NullComparer() {}
};

struct ArrayComparer : ComparerBase {
  const CollectionData* rhs_;

  explicit ArrayComparer(const CollectionData& rhs) : rhs_(&rhs) {}

  CompareResult visitArray(const CollectionData& lhs) {
    if (JsonArrayConst(&lhs) == JsonArrayConst(rhs_))
      return COMPARE_RESULT_EQUAL;
    else
      return COMPARE_RESULT_DIFFER;
  }
};

struct ObjectComparer : ComparerBase {
  const CollectionData* rhs_;

  explicit ObjectComparer(const CollectionData& rhs) : rhs_(&rhs) {}

  CompareResult visitObject(const CollectionData& lhs) {
    if (JsonObjectConst(&lhs) == JsonObjectConst(rhs_))
      return COMPARE_RESULT_EQUAL;
    else
      return COMPARE_RESULT_DIFFER;
  }
};

struct RawComparer : ComparerBase {
  const char* rhsData_;
  size_t rhsSize_;

  explicit RawComparer(const char* rhsData, size_t rhsSize)
      : rhsData_(rhsData), rhsSize_(rhsSize) {}

  CompareResult visitRawJson(const char* lhsData, size_t lhsSize) {
    size_t size = rhsSize_ < lhsSize ? rhsSize_ : lhsSize;
    int n = memcmp(lhsData, rhsData_, size);
    if (n < 0)
      return COMPARE_RESULT_LESS;
    else if (n > 0)
      return COMPARE_RESULT_GREATER;
    else
      return COMPARE_RESULT_EQUAL;
  }
};

struct VariantComparer : ComparerBase {
  const VariantData* rhs;

  explicit VariantComparer(const VariantData* value) : rhs(value) {}

  CompareResult visitArray(const CollectionData& lhs) {
    ArrayComparer comparer(lhs);
    return accept(comparer);
  }

  CompareResult visitObject(const CollectionData& lhs) {
    ObjectComparer comparer(lhs);
    return accept(comparer);
  }

  CompareResult visitFloat(JsonFloat lhs) {
    Comparer<JsonFloat> comparer(lhs);
    return accept(comparer);
  }

  CompareResult visitString(const char* lhs, size_t) {
    Comparer<const char*> comparer(lhs);
    return accept(comparer);
  }

  CompareResult visitRawJson(const char* lhsData, size_t lhsSize) {
    RawComparer comparer(lhsData, lhsSize);
    return accept(comparer);
  }

  CompareResult visitSignedInteger(JsonInteger lhs) {
    Comparer<JsonInteger> comparer(lhs);
    return accept(comparer);
  }

  CompareResult visitUnsignedInteger(JsonUInt lhs) {
    Comparer<JsonUInt> comparer(lhs);
    return accept(comparer);
  }

  CompareResult visitBoolean(bool lhs) {
    Comparer<bool> comparer(lhs);
    return accept(comparer);
  }

  CompareResult visitNull() {
    NullComparer comparer;
    return accept(comparer);
  }

 private:
  template <typename TComparer>
  CompareResult accept(TComparer& comparer) {
    CompareResult reversedResult = variantAccept(rhs, comparer);
    switch (reversedResult) {
      case COMPARE_RESULT_GREATER:
        return COMPARE_RESULT_LESS;
      case COMPARE_RESULT_LESS:
        return COMPARE_RESULT_GREATER;
      default:
        return reversedResult;
    }
  }
};

template <typename T>
struct Comparer<T, typename enable_if<is_convertible<
                       T, ArduinoJson::JsonVariantConst>::value>::type>
    : VariantComparer {
  explicit Comparer(const T& value)
      : VariantComparer(VariantAttorney::getData(value)) {}
};

template <typename T>
CompareResult compare(ArduinoJson::JsonVariantConst lhs, const T& rhs) {
  Comparer<T> comparer(rhs);
  return variantAccept(VariantAttorney::getData(lhs), comparer);
}

ARDUINOJSON_END_PRIVATE_NAMESPACE
