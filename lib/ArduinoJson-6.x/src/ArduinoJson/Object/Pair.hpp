// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2022, Benoit BLANCHON
// MIT License

#pragma once

#include <ArduinoJson/Strings/String.hpp>
#include <ArduinoJson/Variant/VariantRef.hpp>

namespace ARDUINOJSON_NAMESPACE {
// A key value pair for CollectionData.
class Pair {
 public:
  Pair(MemoryPool* pool, VariantSlot* slot) {
    if (slot) {
      _key = String(slot->key(),
                    slot->ownsKey() ? String::Copied : String::Linked);
      _value = VariantRef(pool, slot->data());
    }
  }

  String key() const {
    return _key;
  }

  VariantRef value() const {
    return _value;
  }

 private:
  String _key;
  VariantRef _value;
};

class PairConst {
 public:
  PairConst(const VariantSlot* slot) {
    if (slot) {
      _key = String(slot->key(),
                    slot->ownsKey() ? String::Copied : String::Linked);
      _value = VariantConstRef(slot->data());
    }
  }

  String key() const {
    return _key;
  }

  VariantConstRef value() const {
    return _value;
  }

 private:
  String _key;
  VariantConstRef _value;
};
}  // namespace ARDUINOJSON_NAMESPACE
