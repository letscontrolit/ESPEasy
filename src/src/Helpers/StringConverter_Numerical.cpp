#include "../Helpers/StringConverter_Numerical.h"

#include "../DataStructs/TimingStats.h"

#include "../Helpers/Numerical.h"

#include "../Helpers/StringConverter.h"


/********************************************************************************************\
   Convert a char string to integer
 \*********************************************************************************************/

// FIXME: change original code so it uses String and String.toInt()
unsigned long str2int(const char *string)
{
  uint32_t temp = 0;

  validUIntFromString(string, temp);

  return static_cast<unsigned long>(temp);
}

/*********************************************************************************************\
   Workaround for removing trailing white space when String() converts a float with 0 decimals
\*********************************************************************************************/
String toString(const float& value, unsigned int decimalPlaces, bool trimTrailingZeros)
{
  String res;
  toValidString(res, value, decimalPlaces, trimTrailingZeros);
  return res;
}

bool toValidString(String& str,
  const float& value,
  unsigned int decimalPlaces,
  bool         trimTrailingZeros)
{
  #ifdef ESP32
  if (isnanf(value)) {
    str = F("NaN");
    return false;  
  }
  if (isinff(value)) {
    str = F("Inf");
    return false;
  }
#else
  if (isnan(value)) {
    str = F("NaN");
    return false;
  }
  if (isinf(value)) {
    str = F("Inf");
    return false;
  }
#endif
  const double value_d(value);
  return doubleToValidString(str, value_d, decimalPlaces, trimTrailingZeros);
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
    return concat('-', ull2String(value * -1ll, base));
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

String doubleToString(const double& value, unsigned int decimalPlaces, bool trimTrailingZeros_b) {
  String res;
  doubleToValidString(res, value, decimalPlaces, trimTrailingZeros_b);
  return res;
}


bool  doubleToValidString(String& str,
  const double& value,
  unsigned int  decimalPlaces     ,
  bool          trimTrailingZeros_b)
{
  if (isnan(value)) {
    str = F("NaN");
    return false;
  }
  if (isinf(value)) {
    str = F("Inf");
    return false;
  }
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE

  // We use some trick here to prevent rounding errors 
  // like when representing 23.8, which will be printed like 23.799999...
  // 
  // First calculate factor to represent the value with N decimal places as integer
  // Use maximum of 18 decimals or else the factor will not fit in a 64-bit int
  uint64_t factor = (decimalPlaces > 18) ? 1 : computeDecimalFactorForDecimals(decimalPlaces);

  // Calculate floating point value which could be cast to int64_t later to
  // format the value with N decimal places and later insert the decimal dot.
  const double tmp_value = std::abs(value * factor);
  constexpr double max_uint64 = std::numeric_limits<uint64_t>::max();

  if ((decimalPlaces > 18) || (tmp_value > max_uint64)) {
#endif
    // Cannot use int64_t as intermediate variable
    unsigned int expectedChars = decimalPlaces + 4; // 1 dot, 2 minus signs and terminating zero
    if (value > 1e33 || value < -1e33) {
      expectedChars += 308; // Just assume the worst
    } else {
      expectedChars += 33;
    }
    char *buf = (char *)malloc(expectedChars);

    if (nullptr == buf) {
      str = F("NaN");
      return false;
    }
    move_special(str, String(dtostrf(value, (decimalPlaces + 2), decimalPlaces, buf)));

    free(buf);
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  } else {
    // Round the double value, multiplied with the factor 10^decimalPlaces, 
    // to make sure we will not end up with values like 23.799999...
    uint64_t int_value = round(tmp_value);

    if (trimTrailingZeros_b) {
      while (decimalPlaces > 0 && int_value % 10 == 0) {
        int_value /= 10;
        factor /= 10;
        --decimalPlaces;
      }

      if (decimalPlaces > 2) {
        const uint32_t last2digits = int_value % 100;
        if (last2digits == 99u) {
          ++int_value;
        } else if (last2digits == 1u) {
          --int_value;
        }
      }
    }

    // The value before the decimal point can be larger than what a 32-bit int can represent.
    // Those cannot be used in the string format, so use it as a preformatted string
    const String tmp_str_before_dot = ull2String(int_value / factor);
    if (decimalPlaces == 0) {
      str = tmp_str_before_dot;
    } else {
      String tmp_str_after_dot;
      tmp_str_after_dot.reserve(decimalPlaces);
      tmp_str_after_dot = ull2String(int_value % factor);
      while (tmp_str_after_dot.length() < decimalPlaces) {
        // prepend leading zeroes on the fraction part.
        tmp_str_after_dot = concat('0', tmp_str_after_dot);
      }

      str = strformat(
        F("%s.%s"), 
        tmp_str_before_dot.c_str(), 
        tmp_str_after_dot.c_str());
    }
    if (value < 0) {
      str = concat('-', str);
    }
  }
  #endif

  str.trim();

  if (trimTrailingZeros_b) {
    str = trimTrailingZeros(str);
  }
  return true;
}

String floatToString(const float& value,
                      unsigned int  decimalPlaces,
                      bool          trimTrailingZeros_b)
{
  String res = toString(value, decimalPlaces);

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



/********************************************************************************************\
   Handling HEX strings
 \*********************************************************************************************/

// Convert max. 8 hex decimals to unsigned long
unsigned long hexToUL(const String& input_c, size_t nrHexDecimals) {
  const unsigned long long resULL = hexToULL(input_c, nrHexDecimals);
  return static_cast<unsigned long>(resULL & 0xFFFFFFFFull);
}

unsigned long hexToUL(const String& input_c) {
  return hexToUL(input_c, input_c.length());
}

unsigned long hexToUL(const String& input_c, size_t startpos, size_t nrHexDecimals) {
  return hexToUL(input_c.substring(startpos, startpos + nrHexDecimals), nrHexDecimals);
}

// Convert max. 16 hex decimals to unsigned long long (aka uint64_t)
unsigned long long hexToULL(const String& input_c, size_t nrHexDecimals) {
  size_t nr_decimals = nrHexDecimals;

  if (nr_decimals > 16) {
    nr_decimals = 16;
  }
  const size_t inputLength = input_c.length();

  if (nr_decimals > inputLength) {
    nr_decimals = inputLength;
  } else if (input_c.startsWith(F("0x"))) { // strtoull handles that prefix nicely
    nr_decimals += 2;
  }
  return strtoull(input_c.substring(0, nr_decimals).c_str(), 0, 16);
}

unsigned long long hexToULL(const String& input_c) {
  return hexToULL(input_c, input_c.length());
}

unsigned long long hexToULL(const String& input_c, size_t startpos, size_t nrHexDecimals) {
  return hexToULL(input_c.substring(startpos, startpos + nrHexDecimals), nrHexDecimals);
}

void appendHexChar(uint8_t data, String& string)
{
  const char *hex_chars = "0123456789abcdef";
  string += hex_chars[(data >> 4) & 0xF];
  string += hex_chars[(data) & 0xF];
}

String formatToHex_array(const uint8_t* data, size_t size)
{
  String res;
  res.reserve(2 * size);
  for (size_t i = 0; i < size; ++i) {
    appendHexChar(data[i], res);
  }
  return res;
}

String formatULLtoHex(const uint64_t& value,
                   const __FlashStringHelper * prefix,
                   unsigned int minimal_hex_digits)
{
  return concat(prefix, formatULLtoHex_no_prefix(value, minimal_hex_digits));
}

String formatULLtoHex(const uint64_t& value,
                   const __FlashStringHelper * prefix)
{
  return formatULLtoHex(value, prefix, 0);
}

String formatULLtoHex(const uint64_t& value, unsigned int minimal_hex_digits)
{
  return formatULLtoHex(value, F("0x"), minimal_hex_digits);
}

String formatULLtoHex_no_prefix(const uint64_t& value, unsigned int minimal_hex_digits)
{
  const uint32_t msb_val = static_cast<uint32_t>(value >> 32);
  const uint32_t lsb_val = static_cast<uint32_t>(value & 0xFFFFFFFF);
  String res;
  res.reserve(16);
  res += formatToHex_no_prefix(msb_val, 8);
  res += formatToHex_no_prefix(lsb_val, 8);
  while (minimal_hex_digits < res.length() && res.startsWith(F("0"))) {
    res = res.substring(1);
  }
  return res;
}

String formatULLtoHex_decimal(const uint64_t& value)
{
  return strformat(
    F("%s (%s)"),
    formatULLtoHex(value).c_str(),
    ull2String(value).c_str());
}

String formatToHex_wordarray(const uint16_t* data, size_t size)
{
  String res;
  res.reserve(4 * size);
  for (size_t i = 0; i < size; ++i) {
    appendHexChar(data[i] >> 8, res);
    appendHexChar(data[i] & 0xFF, res);
  }
  return res;
}

String formatToHex(unsigned long value, 
                   const __FlashStringHelper * prefix,
                   unsigned int minimal_hex_digits) {
  return concat(prefix, formatToHex_no_prefix(value, minimal_hex_digits));
}

String formatToHex(unsigned long value,
                   const __FlashStringHelper * prefix) {
  return formatToHex(value, prefix, 0);
}

String formatToHex(unsigned long value, unsigned int minimal_hex_digits) {
  return formatToHex(value, F("0x"), minimal_hex_digits);
}

String formatToHex_no_prefix(unsigned long value, unsigned int minimal_hex_digits) {
  const String fmt = strformat(F("%%0%dX"), minimal_hex_digits);
  return strformat(fmt, value);
}

String formatHumanReadable(uint64_t value,
                           uint32_t factor) {
  String result = formatHumanReadable(value, factor, 2);

  result.replace(F(".00"), EMPTY_STRING);
  return result;
}

String formatHumanReadable(uint64_t value,
                           uint32_t factor, 
                           int NrDecimals) {
  float floatValue(value);
  uint8_t  steps = 0;

  while (value >= factor) {
    value /= factor;
    ++steps;
    floatValue /= float(factor);
  }
  String result = toString(floatValue, NrDecimals);

  if (steps != 0) result += ' ';

  switch (steps) {
    case 0: break;
    case 1: result += 'k'; break;
    case 2: result += 'M'; break;
    case 3: result += 'G'; break;
    case 4: result += 'T'; break;
    default:
      result += '*';
      result += factor;
      result += '^';
      result += steps;
      break;
  }
  return result;
}

String formatToHex_decimal(unsigned long value) {
  return formatToHex_decimal(value, 1);
}

String formatToHex_decimal(unsigned long value, unsigned long factor) {
  String result = formatToHex(value);

  result += F(" (");

  if (factor > 1) {
    result += formatHumanReadable(value, factor);
  } else {
    result += value;
  }
  result += ')';
  return result;
}


int intFromHexChar(char a)
{
  if ((a >= 'A') && (a <= 'F'))      { return a - 'A' + 10; }
  else if ((a >= 'a') && (a <= 'f')) { return a - 'a' + 10; }
  else if ((a >= '0') && (a <= '9')) { return a - '0'; }
  return -1;
}

String stringFromHexArray(const String& arr)
{
  const size_t arr_length = arr.length();
  String res;

  res.reserve(arr_length / 2);
  uint8_t decoded_c{};
  bool    secondNibble = false;

  for (size_t i = 0; i < arr_length; ++i) {
    const char c = arr[i];

    if (isHexadecimalDigit(c)) {
      const int hexChar = intFromHexChar(c) & 0xF;

      if (secondNibble) {
        decoded_c <<= 4;
        decoded_c  += hexChar;
        res        += static_cast<char>(decoded_c);
        secondNibble = false;
      } else {
        decoded_c = hexChar;
        secondNibble = true;
      }
    }
  }
  return res;
}
