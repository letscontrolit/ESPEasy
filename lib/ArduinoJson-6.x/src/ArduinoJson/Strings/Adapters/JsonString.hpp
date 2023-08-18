// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2023, Benoit BLANCHON
// MIT License

#pragma once

#include <ArduinoJson/Strings/Adapters/RamString.hpp>
#include <ArduinoJson/Strings/JsonString.hpp>
#include <ArduinoJson/Strings/StringAdapter.hpp>

ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE

class JsonStringAdapter : public SizedRamString {
 public:
  JsonStringAdapter(const JsonString& s)
      : SizedRamString(s.c_str(), s.size()), linked_(s.isLinked()) {}

  StringStoragePolicy::LinkOrCopy storagePolicy() const {
    StringStoragePolicy::LinkOrCopy policy = {linked_};
    return policy;
  }

 private:
  bool linked_;
};

template <>
struct StringAdapter<JsonString> {
  typedef JsonStringAdapter AdaptedString;

  static AdaptedString adapt(const JsonString& s) {
    return AdaptedString(s);
  }
};

ARDUINOJSON_END_PRIVATE_NAMESPACE
