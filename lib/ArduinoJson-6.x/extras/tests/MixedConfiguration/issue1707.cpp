// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2022, Benoit BLANCHON
// MIT License

#define ARDUINO
#define memcpy_P(dest, src, n) memcpy((dest), (src), (n))

#include <ArduinoJson.h>

#include <catch.hpp>

TEST_CASE("Issue1707") {
  StaticJsonDocument<128> doc;

  DeserializationError err = deserializeJson(doc, F("{\"hello\":12}"));
  REQUIRE(err == DeserializationError::Ok);
}
