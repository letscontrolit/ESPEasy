#ifndef HELPERS_NUMERICAL_H
#define HELPERS_NUMERICAL_H

#include <Arduino.h>

/********************************************************************************************\
  Check if string is valid float
  \*********************************************************************************************/

bool isValidFloat(float f);

bool validIntFromString(const String& tBuf, int& result);

bool validInt64FromString(const String& tBuf, int64_t& result);

bool validUIntFromString(const String& tBuf, unsigned int& result);

bool validUInt64FromString(const String& tBuf, uint64_t& result);

bool validFloatFromString(const String& tBuf, float& result);

bool validDoubleFromString(const String& tBuf, double& result);

enum class NumericalType {
  Integer,
  FloatingPoint,
  HexadecimalUInt
};

String getNumerical(const String& tBuf, NumericalType numericalType, bool& isHex);

bool isNumerical(const String& tBuf, NumericalType numericalType, bool& isHex);


#endif // HELPERS_NUMERICAL_H
