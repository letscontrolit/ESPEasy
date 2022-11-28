// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2022, Benoit BLANCHON
// MIT License

#pragma once

#include <ArduinoJson/Configuration.hpp>
#include <ArduinoJson/Json/JsonSerializer.hpp>
#include <ArduinoJson/Serialization/measure.hpp>
#include <ArduinoJson/Serialization/serialize.hpp>

namespace ARDUINOJSON_NAMESPACE {

template <typename TWriter>
class PrettyJsonSerializer : public JsonSerializer<TWriter> {
  typedef JsonSerializer<TWriter> base;

 public:
  PrettyJsonSerializer(TWriter writer) : base(writer), _nesting(0) {}

  size_t visitArray(const CollectionData& array) {
    const VariantSlot* slot = array.head();
    if (slot) {
      base::write("[\r\n");
      _nesting++;
      while (slot != 0) {
        indent();
        slot->data()->accept(*this);

        slot = slot->next();
        base::write(slot ? ",\r\n" : "\r\n");
      }
      _nesting--;
      indent();
      base::write("]");
    } else {
      base::write("[]");
    }
    return this->bytesWritten();
  }

  size_t visitObject(const CollectionData& object) {
    const VariantSlot* slot = object.head();
    if (slot) {
      base::write("{\r\n");
      _nesting++;
      while (slot != 0) {
        indent();
        base::visitString(slot->key());
        base::write(": ");
        slot->data()->accept(*this);

        slot = slot->next();
        base::write(slot ? ",\r\n" : "\r\n");
      }
      _nesting--;
      indent();
      base::write("}");
    } else {
      base::write("{}");
    }
    return this->bytesWritten();
  }

 private:
  void indent() {
    for (uint8_t i = 0; i < _nesting; i++) base::write(ARDUINOJSON_TAB);
  }

  uint8_t _nesting;
};

template <typename TDestination>
size_t serializeJsonPretty(VariantConstRef source, TDestination& destination) {
  return serialize<PrettyJsonSerializer>(source, destination);
}

inline size_t serializeJsonPretty(VariantConstRef source, void* buffer,
                                  size_t bufferSize) {
  return serialize<PrettyJsonSerializer>(source, buffer, bufferSize);
}

inline size_t measureJsonPretty(VariantConstRef source) {
  return measure<PrettyJsonSerializer>(source);
}

}  // namespace ARDUINOJSON_NAMESPACE
