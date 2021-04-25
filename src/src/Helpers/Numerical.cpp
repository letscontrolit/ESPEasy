#include "Numerical.h"

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
    result = static_cast<int>(tmp);
    return isvalid;
  }
  const bool isvalid = numerical.length() > 0;

  if (isvalid) {
    result = numerical.toInt();
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
    result = atoll(numerical.c_str());
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
    result = strtoul(numerical.c_str(), NULL, base);
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
    result = strtoull(numerical.c_str(), NULL, base);
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
  #ifdef CORE_POST_2_5_0

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
  #else // ifdef CORE_POST_2_5_0
  float tmp = static_cast<float>(result);
  bool  res = validFloatFromString(tBuf, tmp);
  result = static_cast<double>(tmp);
  return res;
  #endif // ifdef CORE_POST_2_5_0
}

bool mustConsiderAsString(NumericalType detectedType) {
  switch (detectedType) {
    case NumericalType::FloatingPoint:
    case NumericalType::Integer:
      break;
    case NumericalType::HexadecimalUInt:
    case NumericalType::BinaryUint:
      return true; // Has '0x' or '0b' as prefix
  }
  return false;
}

bool mustConsiderAsString(const String& value) {
  const unsigned int length = value.length();
  if (length == 0) return true;
  unsigned int i = 0;
  char c = value[i];
  if (c == '+' || c == '-') {
    ++i;
    if (length == i) return true;
    c = value[i];
  }

  bool dotFound = false;
  for (; i < length; ++i) {
    if (c == '.') {
      if (dotFound) {
        return true;
      } else {
        dotFound = true;
      }
    } else {
      if (!isdigit(c)) {
        return true;
      }
    }
    c = value[i];
  }
  return i < length;
}

String getNumerical(const String& tBuf, NumericalType requestedType, NumericalType& detectedType) {
  const unsigned int bufLength = tBuf.length();
  unsigned int firstDec        = 0;
  String result;
  result.reserve(bufLength);

  while (firstDec < bufLength && tBuf.charAt(firstDec) == ' ') {
    ++firstDec;
  }

  if (firstDec >= bufLength) { return result; }
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
      } else if (NumericalType::FloatingPoint == requestedType && c == '.') {
        // Only floating point numbers should start with '0.'
        // All other combinations are not valid.
        ++firstDec;
        result      += c;
        decPt        = true;
        detectedType = NumericalType::FloatingPoint;
      } else {
        return result;
      }
    }
  }

  for (unsigned int x = firstDec; x < bufLength; ++x) {
    c = tBuf.charAt(x);

    if (c == '.') {
      if (NumericalType::FloatingPoint != requestedType) { return result; }

      // Only one decimal point allowed
      if (decPt) { return result; }
      else {
        decPt        = true;
        detectedType = NumericalType::FloatingPoint;
      }
    } else {
      switch (detectedType) {
        case NumericalType::FloatingPoint:
        case NumericalType::Integer:

          if (!isdigit(c)) {
            return result;
          }
          break;
        case NumericalType::HexadecimalUInt:

          if (!isxdigit(c)) {
            return result;
          }
          break;
        case NumericalType::BinaryUint:

          if ((c != '0') && (c != '1')) {
            return result;
          }
          break;
      }
    }
    result += c;
  }
  return result;
}

bool isNumerical(const String& tBuf, NumericalType& detectedType) {
  NumericalType requestedType = NumericalType::FloatingPoint;
  const String  result        = getNumerical(tBuf, requestedType, detectedType);
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
