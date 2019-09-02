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


/*********************************************************************************************\
   Bitwise operators
  \*********************************************************************************************/
static bool getBitFromUL(uint32_t number, byte bitnr);
static void setBitToUL(uint32_t& number, byte bitnr, bool value);

bool getBitFromUL(uint32_t number, byte bitnr) {
  return (number >> bitnr) & 1UL;
}

void setBitToUL(uint32_t& number, byte bitnr, bool value) {
  uint32_t newbit = value ? 1UL : 0UL;
  number ^= (-newbit ^ number) & (1UL << bitnr);
}

#endif // ESPEASY_COMMON_H