// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2020
// MIT License

#include <ArduinoJson.h>
#include <catch.hpp>

using namespace ARDUINOJSON_NAMESPACE;

TEST_CASE("MemberProxy::operator[]") {
  DynamicJsonDocument doc(4096);
  MemberProxy<JsonDocument&, const char*> mp = doc["hello"];

  SECTION("set member") {
    mp["world"] = 42;

    REQUIRE(doc.as<std::string>() == "{\"hello\":{\"world\":42}}");
  }

  SECTION("set element") {
    mp[2] = 42;

    REQUIRE(doc.as<std::string>() == "{\"hello\":[null,null,42]}");
  }
}
