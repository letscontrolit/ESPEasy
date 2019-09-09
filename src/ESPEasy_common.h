#ifndef ESPEASY_COMMON_H
#define ESPEASY_COMMON_H

#include <stddef.h>
namespace std
{
  using ::ptrdiff_t;
  using ::size_t;
}

#include <stdint.h>
#include <Arduino.h>

#define ZERO_FILL(S)  memset((S), 0, sizeof(S))
#define ZERO_TERMINATE(S)  S[sizeof(S) - 1] = 0


String getUnknownString();

/*********************************************************************************************\
   Bitwise operators
  \*********************************************************************************************/
bool getBitFromUL(uint32_t number, byte bitnr);
void setBitToUL(uint32_t& number, byte bitnr, bool value);

#endif // ESPEASY_COMMON_H