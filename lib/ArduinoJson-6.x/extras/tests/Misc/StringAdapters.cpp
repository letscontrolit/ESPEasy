// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2022, Benoit BLANCHON
// MIT License

#define ARDUINOJSON_ENABLE_PROGMEM 1
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1

#include "custom_string.hpp"
#include "weird_strcmp.hpp"

#include <ArduinoJson/Strings/StringAdapters.hpp>

#include <catch.hpp>

using namespace ARDUINOJSON_NAMESPACE;

TEST_CASE("ZeroTerminatedRamString") {
  SECTION("null") {
    ZeroTerminatedRamString s = adaptString(static_cast<const char*>(0));

    CHECK(s.isNull() == true);
    CHECK(s.size() == 0);
  }

  SECTION("non-null") {
    ZeroTerminatedRamString s = adaptString("bravo");

    CHECK(s.isNull() == false);
    CHECK(s.size() == 5);
  }
}

TEST_CASE("SizedRamString") {
  SECTION("null") {
    SizedRamString s = adaptString(static_cast<const char*>(0), 10);

    CHECK(s.isNull() == true);
  }

  SECTION("non-null") {
    SizedRamString s = adaptString("bravo", 5);

    CHECK(s.isNull() == false);
    CHECK(s.size() == 5);
  }
}

TEST_CASE("FlashString") {
  SECTION("null") {
    FlashString s = adaptString(static_cast<const __FlashStringHelper*>(0));

    CHECK(s.isNull() == true);
    CHECK(s.size() == 0);
  }

  SECTION("non-null") {
    FlashString s = adaptString(F("bravo"));

    CHECK(s.isNull() == false);
    CHECK(s.size() == 5);
  }
}

TEST_CASE("std::string") {
  std::string orig("bravo");
  SizedRamString s = adaptString(orig);

  CHECK(s.isNull() == false);
  CHECK(s.size() == 5);
}

TEST_CASE("Arduino String") {
  ::String orig("bravo");
  SizedRamString s = adaptString(orig);

  CHECK(s.isNull() == false);
  CHECK(s.size() == 5);
}

TEST_CASE("custom_string") {
  custom_string orig("bravo");
  SizedRamString s = adaptString(orig);

  CHECK(s.isNull() == false);
  CHECK(s.size() == 5);
}

TEST_CASE("IsString<T>") {
  SECTION("std::string") {
    CHECK(IsString<std::string>::value == true);
  }

  SECTION("basic_string<wchar_t>") {
    CHECK(IsString<std::basic_string<wchar_t> >::value == false);
  }

  SECTION("custom_string") {
    CHECK(IsString<custom_string>::value == true);
  }

  SECTION("const __FlashStringHelper*") {
    CHECK(IsString<const __FlashStringHelper*>::value == true);
  }

  SECTION("const char*") {
    CHECK(IsString<const char*>::value == true);
  }

  SECTION("const char[]") {
    CHECK(IsString<const char[8]>::value == true);
  }
}

TEST_CASE("stringCompare") {
  SECTION("ZeroTerminatedRamString vs ZeroTerminatedRamString") {
    CHECK(stringCompare(adaptString("bravo"), adaptString("alpha")) > 0);
    CHECK(stringCompare(adaptString("bravo"), adaptString("bravo")) == 0);
    CHECK(stringCompare(adaptString("bravo"), adaptString("charlie")) < 0);
  }

  SECTION("ZeroTerminatedRamString vs SizedRamString") {
    CHECK(stringCompare(adaptString("bravo"), adaptString("alpha?", 5)) > 0);
    CHECK(stringCompare(adaptString("bravo"), adaptString("bravo?", 4)) > 0);
    CHECK(stringCompare(adaptString("bravo"), adaptString("bravo?", 5)) == 0);
    CHECK(stringCompare(adaptString("bravo"), adaptString("bravo?", 6)) < 0);
    CHECK(stringCompare(adaptString("bravo"), adaptString("charlie?", 7)) < 0);
  }

  SECTION("SizedRamString vs SizedRamString") {
    // clang-format off
    CHECK(stringCompare(adaptString("bravo!", 5), adaptString("alpha?", 5)) > 0);
    CHECK(stringCompare(adaptString("bravo!", 5), adaptString("bravo?", 5)) == 0);
    CHECK(stringCompare(adaptString("bravo!", 5), adaptString("charlie?", 7)) < 0);

    CHECK(stringCompare(adaptString("bravo!", 5), adaptString("bravo!", 4)) > 0);
    CHECK(stringCompare(adaptString("bravo!", 5), adaptString("bravo!", 5)) == 0);
    CHECK(stringCompare(adaptString("bravo!", 5), adaptString("bravo!", 6)) < 0);
    // clang-format on
  }

  SECTION("FlashString vs FlashString") {
    // clang-format off
    CHECK(stringCompare(adaptString(F("bravo")), adaptString(F("alpha"))) > 0);
    CHECK(stringCompare(adaptString(F("bravo")), adaptString(F("bravo"))) == 0);
    CHECK(stringCompare(adaptString(F("bravo")), adaptString(F("charlie"))) < 0);
    // clang-format on
  }

  SECTION("FlashString vs SizedRamString") {
    // clang-format off
    CHECK(stringCompare(adaptString(F("bravo")), adaptString("alpha?", 5)) > 0);
    CHECK(stringCompare(adaptString(F("bravo")), adaptString("bravo?", 5)) == 0);
    CHECK(stringCompare(adaptString(F("bravo")), adaptString("charlie?", 7)) < 0);

    CHECK(stringCompare(adaptString(F("bravo")), adaptString("bravo!", 4)) > 0);
    CHECK(stringCompare(adaptString(F("bravo")), adaptString("bravo!", 5)) == 0);
    CHECK(stringCompare(adaptString(F("bravo")), adaptString("bravo!", 6)) < 0);
    // clang-format on
  }

  SECTION("ZeroTerminatedRamString vs FlashString") {
    // clang-format off
    CHECK(stringCompare(adaptString("bravo"), adaptString(F("alpha?"), 5)) > 0);
    CHECK(stringCompare(adaptString("bravo"), adaptString(F("bravo?"), 4)) > 0);
    CHECK(stringCompare(adaptString("bravo"), adaptString(F("bravo?"), 5)) == 0);
    CHECK(stringCompare(adaptString("bravo"), adaptString(F("bravo?"), 6)) < 0);
    CHECK(stringCompare(adaptString("bravo"), adaptString(F("charlie?"), 7)) < 0);
    // clang-format on
  }
}

TEST_CASE("stringEquals()") {
  SECTION("ZeroTerminatedRamString vs ZeroTerminatedRamString") {
    CHECK(stringEquals(adaptString("bravo"), adaptString("brav")) == false);
    CHECK(stringEquals(adaptString("bravo"), adaptString("bravo")) == true);
    CHECK(stringEquals(adaptString("bravo"), adaptString("bravo!")) == false);
  }

  SECTION("ZeroTerminatedRamString vs SizedRamString") {
    // clang-format off
    CHECK(stringEquals(adaptString("bravo"), adaptString("bravo!", 4)) == false);
    CHECK(stringEquals(adaptString("bravo"), adaptString("bravo!", 5)) == true);
    CHECK(stringEquals(adaptString("bravo"), adaptString("bravo!", 6)) == false);
    // clang-format on
  }

  SECTION("FlashString vs SizedRamString") {
    // clang-format off
    CHECK(stringEquals(adaptString(F("bravo")), adaptString("bravo!", 4)) == false);
    CHECK(stringEquals(adaptString(F("bravo")), adaptString("bravo!", 5)) == true);
    CHECK(stringEquals(adaptString(F("bravo")), adaptString("bravo!", 6)) == false);
    // clang-format on
  }

  SECTION("SizedRamString vs SizedRamString") {
    // clang-format off
    CHECK(stringEquals(adaptString("bravo?", 5), adaptString("bravo!", 4)) == false);
    CHECK(stringEquals(adaptString("bravo?", 5), adaptString("bravo!", 5)) == true);
    CHECK(stringEquals(adaptString("bravo?", 5), adaptString("bravo!", 6)) == false);
    // clang-format on
  }
}
