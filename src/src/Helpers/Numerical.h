#ifndef HELPERS_NUMERICAL_H
#define HELPERS_NUMERICAL_H

#include <Arduino.h>

/********************************************************************************************\
  Check if string is valid float
  \*********************************************************************************************/
bool isFloat(const String& tBuf);

bool isValidFloat(float f);

bool isInt(const String& tBuf);

bool validIntFromString(const String& tBuf, int& result);

bool validUIntFromString(const String& tBuf, unsigned int& result);

bool validFloatFromString(const String& tBuf, float& result);

bool validDoubleFromString(const String& tBuf, double& result);

String getNumerical(const String& tBuf, bool mustBeInteger);

bool isNumerical(const String& tBuf, bool mustBeInteger);

#endif // HELPERS_NUMERICAL_H
