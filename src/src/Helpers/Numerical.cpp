#include "Numerical.h"

/********************************************************************************************\
  Check if string is valid float
  \*********************************************************************************************/
bool isFloat(const String& tBuf) {
  return isNumerical(tBuf, NumericalType::FloatingPoint);
}

bool isValidFloat(float f) {
  if (isnan(f))      return false; //("isnan");
  if (isinf(f))      return false; //("isinf");
  return true;
}

bool isInt(const String& tBuf) {
  return isNumerical(tBuf, NumericalType::Integer);
}

bool isHex(const String& tBuf) {
  return isNumerical(tBuf, NumericalType::HexadecimalUInt);
}

bool validIntFromString(const String& tBuf, int& result) {
  const String numerical = getNumerical(tBuf, NumericalType::Integer);
  const bool isvalid = numerical.length() > 0;
  if (isvalid) {
    result = numerical.toInt();
  }
  return isvalid;
}

bool validInt64FromString(const String& tBuf, int64_t& result) {
  const String numerical = getNumerical(tBuf, NumericalType::Integer);
  const bool isvalid = numerical.length() > 0;
  if (isvalid) {
    result = atoll(numerical.c_str());
  }
  return isvalid;
}

bool validUIntFromString(const String& tBuf, unsigned int& result) {
  String numerical = getNumerical(tBuf, NumericalType::HexadecimalUInt);
  const bool isvalid = numerical.length() > 0;
  if (isvalid) {
    int base = DEC;
    if (numerical.startsWith(F("0x"))) {
      numerical = numerical.substring(2);
      base = HEX;
    }
    result = strtoul(numerical.c_str(), NULL, base);
  }
  return isvalid;
}

bool validUInt64FromString(const String& tBuf, uint64_t& result) {
  String numerical = getNumerical(tBuf, NumericalType::HexadecimalUInt);
  const bool isvalid = numerical.length() > 0;
  if (isvalid) {
    int base = DEC;
    if (numerical.startsWith(F("0x"))) {
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
  const String numerical = getNumerical(tBuf, NumericalType::FloatingPoint);
  const bool isvalid = numerical.length() > 0;
  if (isvalid) {
    result = numerical.toFloat();
  }
  return isvalid;
}

bool validDoubleFromString(const String& tBuf, double& result) {
  #ifdef CORE_POST_2_5_0
  // String.toDouble() is introduced in core 2.5.0
  const String numerical = getNumerical(tBuf, NumericalType::FloatingPoint);
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


String getNumerical(const String& tBuf, NumericalType numericalType) {
  String result;
  const unsigned int bufLength = tBuf.length();
  unsigned int firstDec = 0;
  while (firstDec < bufLength && tBuf.charAt(firstDec) == ' ') {
    ++firstDec;
  }
  if (firstDec >= bufLength) return result;

  bool decPt = false;
  
  char c = tBuf.charAt(firstDec);
  switch (numericalType) {
    case NumericalType::FloatingPoint:
    case NumericalType::Integer:
      if(c == '+' || c == '-') {
        result += c;
        ++firstDec;
      }
      break;
    case NumericalType::HexadecimalUInt:
      {
        if (c == '0') {
          ++firstDec;
          result += c;
          if (firstDec < bufLength) {
            c = tBuf.charAt(firstDec);
            if (c == 'x') {
              ++firstDec;
              result += c;
            }
          }
        }
      }
      break;
  }

  for(unsigned int x=firstDec; x < bufLength; ++x) {
    c = tBuf.charAt(x);
    if(c == '.') {
      if (NumericalType::FloatingPoint != numericalType) return result;
      // Only one decimal point allowed
      if(decPt) return result;
      else decPt = true;
    }
    switch (numericalType) {
      case NumericalType::FloatingPoint:
      case NumericalType::Integer:
        if (!isdigit(c)) {
          return result;
        }
        break;
      case NumericalType::HexadecimalUInt:
        {
          if (!isxdigit(c)) {
            return result;
          }
        }
        break;
    }
    result += c;
  }
  return result;
}

bool isNumerical(const String& tBuf, NumericalType numericalType) {
  const unsigned int bufLength = tBuf.length();
  unsigned int firstDec = 0;
  while (firstDec < bufLength && tBuf.charAt(firstDec) == ' ') {
    ++firstDec;
  }
  if (firstDec >= bufLength) return false;
  bool decPt = false;
  char c = tBuf.charAt(firstDec);

  switch (numericalType) {
    case NumericalType::FloatingPoint:
    case NumericalType::Integer:
      if(c == '+' || c == '-') {
        ++firstDec;
      }
      break;
    case NumericalType::HexadecimalUInt:
      {
        if (c == '0') {
          ++firstDec;
          if (firstDec < bufLength) {
            c = tBuf.charAt(firstDec);
            if (c == 'x') {
              ++firstDec;
            }
          }
        }
      }
      break;
  }
  
  for(unsigned int x=firstDec; x < bufLength; ++x) {
    c = tBuf.charAt(x);
    if(c == '.') {
      if (NumericalType::FloatingPoint != numericalType) return false;
      // Only one decimal point allowed
      if(decPt) return false;
      else decPt = true;
    }
    else {
      switch (numericalType) {
        case NumericalType::FloatingPoint:
        case NumericalType::Integer:
          if (!isdigit(c)) {
            return false;
          }
          break;
        case NumericalType::HexadecimalUInt:
          {
            if (!isxdigit(c)) {
              return false;
            }
          }
          break;
      }
    }
  }
  return true;
}


