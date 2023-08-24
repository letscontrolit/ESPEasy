#include "../Helpers/StringConverter_Numerical.h"

#include "../Helpers/Numerical.h"

/********************************************************************************************\
   Convert a char string to integer
 \*********************************************************************************************/

// FIXME: change original code so it uses String and String.toInt()
unsigned long str2int(const char *string)
{
  unsigned int temp = 0;

  validUIntFromString(string, temp);

  return static_cast<unsigned long>(temp);
}

/*********************************************************************************************\
   Workaround for removing trailing white space when String() converts a float with 0 decimals
\*********************************************************************************************/
String toString(const float& value, unsigned int decimalPlaces)
{
  /*
  #ifndef LIMIT_BUILD_SIZE

  if (decimalPlaces == 0) {
    if ((value > -2e9f) && (value < 2e9f)) {
      const int32_t l_value = static_cast<int32_t>(roundf(value));
      return String(l_value);
    }
    if ((value > -1e18f) && (value < 1e18f)) {
      // Work-around to perform a faster conversion
      const int64_t ll_value = static_cast<int64_t>(roundf(value));
      return ll2String(ll_value);
    }
  }
  #endif // ifndef LIMIT_BUILD_SIZE
  */
// #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  // This has been fixed in ESP32 code, not (yet) in ESP8266 code
  // https://github.com/espressif/arduino-esp32/pull/6138/files
  //  #ifdef ESP8266
  char buf[decimalPlaces + 42];
  String sValue(dtostrf(value, (decimalPlaces + 2), decimalPlaces, buf));

/*
#else
  String sValue = String(value, decimalPlaces);
#endif
*/
  sValue.trim();
  return sValue;
}

String ull2String(uint64_t value, uint8_t base) {
  String res;

  if (value == 0) {
    res = '0';
    return res;
  }

  while (value > 0) {
    res   += String(static_cast<uint32_t>(value % base), base);
    value /= base;
  }

  int endpos   = res.length() - 1;
  int beginpos = 0;

  while (endpos > beginpos) {
    const char c = res[beginpos];
    res[beginpos] = res[endpos];
    res[endpos]   = c;
    ++beginpos;
    --endpos;
  }

  return res;
}

String ll2String(int64_t value, uint8_t  base) {
  if (value < 0) {
    String res;
    res  = '-';
    res += ull2String(value * -1ll, base);
    return res;
  } else {
    return ull2String(value, base);
  }
}

String trimTrailingZeros(const String& value) {
  String res(value);
  int dot_pos = res.lastIndexOf('.');

  if (dot_pos != -1) {
    bool someTrimmed = false;

    for (int i = res.length() - 1; i > dot_pos && res[i] == '0'; --i) {
      someTrimmed = true;
      res[i]      = ' ';
    }

    if (someTrimmed) {
      res.trim();
    }

    if (res.endsWith(F("."))) {
      res[dot_pos] = ' ';
      res.trim();
    }
  }
  return res;

}

/**
 * Helper: Convert an integer to string, but return an empty string for 0, to save a little space in settings
 */
String toStringNoZero(int64_t value) {
  if (value != 0) {
    return ll2String(value);
  } else {
    return EMPTY_STRING;
  }
}

#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
String doubleToString(const double& value, unsigned int decimalPlaces, bool trimTrailingZeros_b) {
  // This has been fixed in ESP32 code, not (yet) in ESP8266 code
  // https://github.com/espressif/arduino-esp32/pull/6138/files
  //  #ifdef ESP8266
  unsigned int expectedChars = decimalPlaces + 4; // 1 dot, 2 minus signs and terminating zero

  if ((value > 1e32) || (value < -1e32)) {
    expectedChars += 308;                         // Just assume the worst
  } else {
    expectedChars += 33;
  }
  char *buf = (char *)malloc(expectedChars);

  if (nullptr == buf) {
    return F("nan");
  }
  String res(dtostrf(value, (decimalPlaces + 2), decimalPlaces, buf));

  free(buf);

  //  #else
  //  String res(value, decimalPlaces);
  //  #endif
  res.trim();

  if (trimTrailingZeros_b) {
    return trimTrailingZeros(res);
  }
  return res;
}
#endif

String floatToString(const float& value,
                      unsigned int  decimalPlaces,
                      bool          trimTrailingZeros_b)
{
  const String res = toString(value, decimalPlaces);

  if (trimTrailingZeros_b) {
    return trimTrailingZeros(res);
  }
  return res;
}


/********************************************************************************************\
   Check if valid float and convert string to float.
 \*********************************************************************************************/
bool string2float(const String& string, float& floatvalue) {
  return validFloatFromString(string, floatvalue);
}
