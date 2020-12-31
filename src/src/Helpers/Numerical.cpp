#include "Numerical.h"

/********************************************************************************************\
  Check if string is valid float
  \*********************************************************************************************/

bool isValidFloat(float f) {
  if (isnan(f))      return false; //("isnan");
  if (isinf(f))      return false; //("isinf");
  return true;
}

bool validIntFromString(const String& tBuf, int& result) {
  bool isHex;
  const String numerical = getNumerical(tBuf, NumericalType::Integer, isHex);
  if (isHex) {
    unsigned int tmp;
    bool isvalid = validUIntFromString(tBuf, tmp);
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
  bool isHex;
  const String numerical = getNumerical(tBuf, NumericalType::Integer, isHex);
  if (isHex) {
    uint64_t tmp;
    bool isvalid = validUInt64FromString(tBuf, tmp);
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
  bool isHex;
  String numerical = getNumerical(tBuf, NumericalType::HexadecimalUInt, isHex);
  const bool isvalid = numerical.length() > 0;
  if (isvalid) {
    int base = DEC;
    if (isHex) {
      numerical = numerical.substring(2);
      base = HEX;
    }
    result = strtoul(numerical.c_str(), NULL, base);
  }
  return isvalid;
}

bool validUInt64FromString(const String& tBuf, uint64_t& result) {
  bool isHex;
  String numerical = getNumerical(tBuf, NumericalType::HexadecimalUInt, isHex);
  const bool isvalid = numerical.length() > 0;
  if (isvalid) {
    int base = DEC;
    if (isHex) {
      numerical = numerical.substring(2);
      base = HEX;
    }
    result = strtoull(numerical.c_str(), NULL, base);
  }
  return isvalid;
}

bool validFloatFromString(const String& tBuf, float& result) {
  // DO not call validDoubleFromString and then cast to float.
  // Working with double values is quite CPU intensive as it must be done in software 
  // since the ESP does not have large enough registers for handling double values in hardware.
  bool isHex;
  const String numerical = getNumerical(tBuf, NumericalType::FloatingPoint, isHex);
  if (isHex) {
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
  bool isHex;
  const String numerical = getNumerical(tBuf, NumericalType::FloatingPoint, isHex);
  if (isHex) {
    uint64_t tmp;
    bool isvalid = validUInt64FromString(tBuf, tmp);
    result = static_cast<double>(tmp);
    return isvalid;
  }

  const bool isvalid = numerical.length() > 0;
  if (isvalid) {
    result = numerical.toDouble();
  }
  return isvalid;
  #else
  float tmp = static_cast<float>(result);
  bool res = validFloatFromString(tBuf, tmp);
  result = static_cast<double>(tmp);
  return res;
  #endif
}


String getNumerical(const String& tBuf, NumericalType numericalType, bool& isHex) {
  String result;
  const unsigned int bufLength = tBuf.length();
  unsigned int firstDec = 0;
  while (firstDec < bufLength && tBuf.charAt(firstDec) == ' ') {
    ++firstDec;
  }
  if (firstDec >= bufLength) return result;
  bool decPt = false;
  isHex = false;
  char c = tBuf.charAt(firstDec);
  if (c == '+' || c == '-') {
    if (numericalType != NumericalType::HexadecimalUInt) {
      result += c;
      ++firstDec;
    }
  } else if (c == '0') {
    ++firstDec;
    result += c;
    if (firstDec < bufLength) {
      c = tBuf.charAt(firstDec);
      if (c == 'x') {
        ++firstDec;
        result += c;
        isHex = true;
      }
    }
  }

  for(unsigned int x=firstDec; x < bufLength; ++x) {
    c = tBuf.charAt(x);
    if(c == '.') {
      if (NumericalType::FloatingPoint != numericalType) return result;
      // Only one decimal point allowed
      if(decPt) return result;
      else decPt = true;
    } else {
      if (isHex) {
        if (!isxdigit(c)) {
          return result;
        }
      } else {
        if (!isdigit(c)) {
          return result;
        }
      }
    }
    result += c;
  }
  return result;
}

bool isNumerical(const String& tBuf, NumericalType numericalType, bool& isHex) {
  const unsigned int bufLength = tBuf.length();
  unsigned int firstDec = 0;
  while (firstDec < bufLength && tBuf.charAt(firstDec) == ' ') {
    ++firstDec;
  }
  if (firstDec >= bufLength) return false;
  bool decPt = false;
  isHex = false;
  char c = tBuf.charAt(firstDec);
  if (c == '+' || c == '-') {
    if (numericalType != NumericalType::HexadecimalUInt) {
      ++firstDec;
    }
  } else if (c == '0') {
    ++firstDec;
    if (firstDec < bufLength) {
      c = tBuf.charAt(firstDec);
      if (c == 'x') {
        ++firstDec;
        isHex = true;
      }
    }
  }
  
  for(unsigned int x=firstDec; x < bufLength; ++x) {
    c = tBuf.charAt(x);
    if(c == '.') {
      if (NumericalType::FloatingPoint != numericalType) return false;
      // Only one decimal point allowed
      if(decPt) return false;
      else decPt = true;
    } else {
      if (isHex) {
        if (!isxdigit(c)) {
          return false;
        }
      } else {
        if (!isdigit(c)) {
          return false;
        }
      }
    }
  }
  return true;
}


