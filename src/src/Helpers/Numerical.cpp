#include "Numerical.h"

/********************************************************************************************\
  Check if string is valid float
  \*********************************************************************************************/
bool isFloat(const String& tBuf) {
  return isNumerical(tBuf, false);
}

bool isValidFloat(float f) {
  if (isnan(f))      return false; //("isnan");
  if (isinf(f))      return false; //("isinf");
  return true;
}

bool isInt(const String& tBuf) {
  return isNumerical(tBuf, true);
}

bool validIntFromString(const String& tBuf, int& result) {
  const String numerical = getNumerical(tBuf, true);
  const bool isvalid = numerical.length() > 0;
  if (isvalid) {
    result = numerical.toInt();
  }
  return isvalid;
}

bool validUIntFromString(const String& tBuf, unsigned int& result) {
  int tmp;
  if (!validIntFromString(tBuf, tmp)) return false;
  if (tmp < 0) return false;
  result = static_cast<unsigned int>(tmp);
  return true;
}


bool validFloatFromString(const String& tBuf, float& result) {
  // DO not call validDoubleFromString and then cast to float.
  // Working with double values is quite CPU intensive as it must be done in software 
  // since the ESP does not have large enough registers for handling double values in hardware.
  const String numerical = getNumerical(tBuf, false);
  const bool isvalid = numerical.length() > 0;
  if (isvalid) {
    result = numerical.toFloat();
  }
  return isvalid;
}

bool validDoubleFromString(const String& tBuf, double& result) {
  #ifdef CORE_POST_2_5_0
  // String.toDouble() is introduced in core 2.5.0
  const String numerical = getNumerical(tBuf, false);
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


String getNumerical(const String& tBuf, bool mustBeInteger) {
  String result = "";
  const unsigned int bufLength = tBuf.length();
  unsigned int firstDec = 0;
  while (firstDec < bufLength && tBuf.charAt(firstDec) == ' ') {
    ++firstDec;
  }
  if (firstDec >= bufLength) return result;

  bool decPt = false;
  
  char c = tBuf.charAt(firstDec);
  if(c == '+' || c == '-') {
    result += c;
    ++firstDec;
  }
  for(unsigned int x=firstDec; x < bufLength; ++x) {
    c = tBuf.charAt(x);
    if(c == '.') {
      if (mustBeInteger) return result;
      // Only one decimal point allowed
      if(decPt) return result;
      else decPt = true;
    }
    else if(c < '0' || c > '9') return result;
    result += c;
  }
  return result;
}

bool isNumerical(const String& tBuf, bool mustBeInteger) {
  const unsigned int bufLength = tBuf.length();
  unsigned int firstDec = 0;
  while (firstDec < bufLength && tBuf.charAt(firstDec) == ' ') {
    ++firstDec;
  }
  if (firstDec >= bufLength) return false;
  bool decPt = false;
  char c = tBuf.charAt(firstDec);
  if(c == '+' || c == '-')
    ++firstDec;
  for(unsigned int x=firstDec; x < bufLength; ++x) {
    c = tBuf.charAt(x);
    if(c == '.') {
      if (mustBeInteger) return false;
      // Only one decimal point allowed
      if(decPt) return false;
      else decPt = true;
    }
    else if(c < '0' || c > '9') return false;
  }
  return true;
}
