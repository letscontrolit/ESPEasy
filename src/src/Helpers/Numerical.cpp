#include "../Helpers/Numerical.h"

#include "../Globals/Settings.h"
#include "../Helpers/StringConverter.h"

/********************************************************************************************\
   Check if string is valid float
 \*********************************************************************************************/
bool isValidFloat(float f) {
  if (isnan(f)) { return false; // ("isnan");
  }

  if (isinf(f)) { return false; // ("isinf");
  }
  return true;
}

bool validIntFromString(const String& tBuf, int& result) {
  NumericalType detectedType;
  const String  numerical = getNumerical(tBuf, NumericalType::Integer, detectedType);

  if ((detectedType == NumericalType::BinaryUint) ||
      (detectedType == NumericalType::HexadecimalUInt)) {
    unsigned int tmp;
    bool isvalid = validUIntFromString(numerical, tmp);

    // FIXME TD-er: What to do here if the uint value > max_int ?
    result = static_cast<int>(tmp);
    return isvalid;
  }
  const bool isvalid = numerical.length() > 0;

  if (isvalid) {
    result = strtol(numerical.c_str(), nullptr, DEC);
  }
  return isvalid;
}

bool validInt64FromString(const String& tBuf, int64_t& result) {
  NumericalType detectedType;
  const String  numerical = getNumerical(tBuf, NumericalType::Integer, detectedType);

  if ((detectedType == NumericalType::BinaryUint) ||
      (detectedType == NumericalType::HexadecimalUInt)) {
    uint64_t tmp;
    bool     isvalid = validUInt64FromString(numerical, tmp);
    result = static_cast<int64_t>(tmp);
    return isvalid;
  }

  const bool isvalid = numerical.length() > 0;

  if (isvalid) {
    result = strtoll(numerical.c_str(), nullptr, DEC);
  }
  return isvalid;
}

bool validUIntFromString(const String& tBuf, unsigned int& result) {
  NumericalType detectedType;
  String numerical   = getNumerical(tBuf, NumericalType::HexadecimalUInt, detectedType);
  const bool isvalid = numerical.length() > 0;

  if (isvalid) {
    int base = DEC;

    if (detectedType == NumericalType::HexadecimalUInt) {
      numerical = numerical.substring(2);
      base      = HEX;
    } else if (detectedType == NumericalType::BinaryUint) {
      numerical = numerical.substring(2);
      base      = BIN;
    }
    result = strtoul(numerical.c_str(), nullptr, base);
  }
  return isvalid;
}

bool validUInt64FromString(const String& tBuf, uint64_t& result) {
  NumericalType detectedType;
  String numerical   = getNumerical(tBuf, NumericalType::HexadecimalUInt, detectedType);
  const bool isvalid = numerical.length() > 0;

  if (isvalid) {
    int base = DEC;

    if (detectedType == NumericalType::HexadecimalUInt) {
      numerical = numerical.substring(2);
      base      = HEX;
    } else if (detectedType == NumericalType::BinaryUint) {
      numerical = numerical.substring(2);
      base      = BIN;
    }
    result = strtoull(numerical.c_str(), nullptr, base);
  }
  return isvalid;
}

bool validFloatFromString(const String& tBuf, float& result) {
  // DO not call validDoubleFromString and then cast to float.
  // Working with double values is quite CPU intensive as it must be done in software
  // since the ESP does not have large enough registers for handling double values in hardware.
  NumericalType detectedType;
  const String  numerical = getNumerical(tBuf, NumericalType::FloatingPoint, detectedType);

  if ((detectedType == NumericalType::BinaryUint) ||
      (detectedType == NumericalType::HexadecimalUInt)) {
    unsigned int tmp;
    bool isvalid = validUIntFromString(tBuf, tmp);
    result = static_cast<float>(tmp);
    return isvalid;
  }

  const bool isvalid = numerical.length() > 0;

  if (isvalid) {
    result = numerical.toFloat();
  }
  return isvalid;
}

bool validDoubleFromString(const String& tBuf, double& result) {
  #if defined(CORE_POST_2_5_0) || defined(ESP32)

  // String.toDouble() is introduced in core 2.5.0
  NumericalType detectedType;
  const String  numerical = getNumerical(tBuf, NumericalType::FloatingPoint, detectedType);

  if ((detectedType == NumericalType::BinaryUint) ||
      (detectedType == NumericalType::HexadecimalUInt)) {
    uint64_t tmp;
    bool     isvalid = validUInt64FromString(tBuf, tmp);
    result = static_cast<double>(tmp);
    return isvalid;
  }

  const bool isvalid = numerical.length() > 0;

  if (isvalid) {
    result = numerical.toDouble();
  }
  return isvalid;
  #else // if defined(CORE_POST_2_5_0) || defined(ESP32)
  float tmp = static_cast<float>(result);
  bool  res = validFloatFromString(tBuf, tmp);
  result = static_cast<double>(tmp);
  return res;
  #endif // if defined(CORE_POST_2_5_0) || defined(ESP32)
}

bool mustConsiderAsString(NumericalType detectedType) {
  switch (detectedType) {
    case NumericalType::FloatingPoint:
    case NumericalType::Integer:
      break;
    case NumericalType::HexadecimalUInt:
    case NumericalType::BinaryUint: // Has '0x' or '0b' as prefix
    case NumericalType::Not_a_number:
      return true;
  }
  return false;
}

bool mustConsiderAsJSONString(const String& value) {
  if (value.isEmpty()) {
    // Empty string
    return true;
  }

  NumericalType detectedType;
  const bool    isNum  = isNumerical(value, detectedType);
  const bool    isBool = (Settings.JSONBoolWithoutQuotes() && ((value.equalsIgnoreCase(F("true")) || value.equalsIgnoreCase(F("false")))));

  return !isBool && (!isNum || value.isEmpty() || mustConsiderAsString(detectedType));
}

String getNumerical(const String& tBuf, NumericalType requestedType, NumericalType& detectedType) {
  const unsigned int bufLength = tBuf.length();
  unsigned int firstDec        = 0;
  String result;

  // Strip leading spaces
  while (firstDec < bufLength && tBuf.charAt(firstDec) == ' ') {
    ++firstDec;
  }

  if (firstDec >= bufLength) {
    detectedType = NumericalType::Not_a_number;
    return result;
  }
  bool decPt = false;

  detectedType = NumericalType::Integer;
  char c = tBuf.charAt(firstDec);

  if ((c == '+') || (c == '-')) {
    if ((requestedType != NumericalType::HexadecimalUInt) &&
        (requestedType != NumericalType::BinaryUint)) {
      if (c == '-') {
        result += c;
      }
      ++firstDec;

      if (firstDec < bufLength) {
        c = tBuf.charAt(firstDec);
      }
    }
  }

  // Strip leading zeroes
  while (c == '0' && 
         (firstDec + 1) < bufLength && 
         isdigit(tBuf.charAt(firstDec + 1))) {
    ++firstDec;
    c = tBuf.charAt(firstDec);
  }

  if (c == '0') {
    ++firstDec;
    result += c;

    if (firstDec < bufLength) {
      c = tBuf.charAt(firstDec);

      if ((c == 'x') || (c == 'X')) {
        ++firstDec;
        result      += c;
        detectedType = NumericalType::HexadecimalUInt;
      } else if ((c == 'b') || (c == 'B')) {
        ++firstDec;
        result      += c;
        detectedType = NumericalType::BinaryUint;
      } else if (NumericalType::Integer == requestedType) {
        // Allow leading zeroes in Integer types (e.g. in time notation)
        while (c == '0' && firstDec < bufLength) {
          // N.B. intentional "reverse order" of reading char and ++firstDec
          c = tBuf.charAt(firstDec);
          ++firstDec;
        }
      } else if ((NumericalType::FloatingPoint == requestedType) && (c == '.')) {
        // Only floating point numbers should start with '0.'
        // All other combinations are not valid.
        ++firstDec;
        result      += c;
        decPt        = true;
        detectedType = NumericalType::FloatingPoint;
      } else {
        if (equals(result, '-')) {
          detectedType = NumericalType::Not_a_number;
          return emptyString;
        }
        return result;
      }
    }
  } else {
    // Does not start with a 0 and already tested for +/-
    // Only allowed to have a '.' or digits.
    if ((c != '.') && !isdigit(c)) {
      detectedType = NumericalType::Not_a_number;
      return result;
    }
  }

  bool done = false;

  result.reserve(bufLength - firstDec + result.length());

  for (unsigned int x = firstDec; !done && x < bufLength; ++x) {
    c = tBuf.charAt(x);

    if (c == '.') {
      if (NumericalType::FloatingPoint != requestedType) { done = true; }

      // Only one decimal point allowed
      if (decPt) { done = true; }
      else {
        decPt        = true;
        detectedType = NumericalType::FloatingPoint;
      }
    } else {
      switch (detectedType) {
        case NumericalType::FloatingPoint:
        case NumericalType::Integer:

          if (!isdigit(c)) {
            done = true;
          }
          break;
        case NumericalType::HexadecimalUInt:

          if (!isxdigit(c)) {
            done = true;
          }
          break;
        case NumericalType::BinaryUint:

          if ((c != '0') && (c != '1')) {
            done = true;
          }
          break;
        case NumericalType::Not_a_number:
          done = true;
          break;
      }
    }

    if (!done) {
      result += c;
    }
  }

  if (equals(result, '-')) {
    detectedType = NumericalType::Not_a_number;
    return emptyString;
  }
  return result;
}

bool isNumerical(const String& tBuf, NumericalType& detectedType) {
  NumericalType requestedType = NumericalType::FloatingPoint;
  const String  result        = getNumerical(tBuf, requestedType, detectedType);

  if (detectedType == NumericalType::Not_a_number) { return false; }

  if (result.length() > 0)
  {
    String tmp(tBuf);
    tmp.trim(); // remove leading and trailing spaces

    // Resulting size should be the same size as the given string.
    // Not sure if it is possible to have a longer result, but better be sure to also allow for larger resulting strings.
    // For example ".123" -> "0.123"
    return result.length() >= tmp.length();
  }


  return result.length() > 0;
}
