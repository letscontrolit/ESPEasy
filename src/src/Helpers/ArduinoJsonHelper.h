#ifndef HELPERS_ARDUINOJSONHELPER_H
#define HELPERS_ARDUINOJSONHELPER_H

#include <ArduinoJson.h>

/*
   Wrapper class for ArduinoJson template code.
   Since the ArduinoJson library is completely written as templates,
   each instance of such a class will end up as a new implementation in the compiled binary.

   Also each return type, which are essentially the same, like int / long
   will be considered a different type for the compiler.

 */

struct ArduinoJsonHelper_t {
  // Empty wrapper, user needs to call resetMemoryPool() before using
  ArduinoJsonHelper_t();

  // Create a DynamicJsonDocument with given expected json length.
  ArduinoJsonHelper_t(size_t jsonLength);
  ~ArduinoJsonHelper_t();

  // clears the JsonDocument and releases all the memory from the memory pool.
  void                 clear();

  bool                 resetMemoryPool(size_t jsonLength);

  DeserializationError do_deserializeJson(const String& json);

  bool                 isNull() const;

  int                  getInt(const __FlashStringHelper *key) const;
  int                  getInt(const String& key) const;

  float                getFloat(const __FlashStringHelper *key) const;
  float                getFloat(const String& key) const;

  String               getString(const __FlashStringHelper *key,
                                 const __FlashStringHelper *default_str) const;
  String               getString(const __FlashStringHelper *key,
                                 const String             & default_str) const;
  String               getString(const String& key,
                                 const String& default_str) const;

  JsonObject           getDoc();

  size_t               getJsonLength() const {
    return _jsonLength;
  }

private:

  DynamicJsonDocument *root        = nullptr;
  size_t               _jsonLength = 0;
};
#endif // ifndef HELPERS_ARDUINOJSONHELPER_H
