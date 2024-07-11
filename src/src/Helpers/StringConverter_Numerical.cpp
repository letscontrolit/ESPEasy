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
  const double value_d(value);
  return doubleToString(value_d, decimalPlaces, trimTrailingZeros);
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
    // Cannot use int64_t as intermediate variable
    unsigned int expectedChars = decimalPlaces + 4; // 1 dot, 2 minus signs and terminating zero
    if (value > 1e33 || value < -1e33) {
      expectedChars += 308; // Just assume the worst
    } else {
      expectedChars += 33;
    }
    char *buf = (char *)malloc(expectedChars);

    if (nullptr == buf) {
      return F("nan");
    }
    move_special(res, String(dtostrf(value, (decimalPlaces + 2), decimalPlaces, buf)));

    free(buf);
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
      res = tmp_str_before_dot;
    } else {
      String tmp_str_after_dot;
      tmp_str_after_dot.reserve(decimalPlaces);
      tmp_str_after_dot = ull2String(int_value % factor);
      while (tmp_str_after_dot.length() < decimalPlaces) {
        // prepend leading zeroes on the fraction part.
        tmp_str_after_dot = concat('0', tmp_str_after_dot);
      }

      res = strformat(
        F("%s.%s"), 
        tmp_str_before_dot.c_str(), 
        tmp_str_after_dot.c_str());
    }
    if (value < 0) {
      res = concat('-', res);
    }
  }

  res.trim();

  if (trimTrailingZeros_b) {
    return trimTrailingZeros(res);
  }
  return res;
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
