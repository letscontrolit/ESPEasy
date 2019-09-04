#include "ESPEasy_common.h"


String getUnknownString() { return F("Unknown"); }

/*********************************************************************************************\
   Bitwise operators
  \*********************************************************************************************/
bool getBitFromUL(uint32_t number, byte bitnr) {
  return (number >> bitnr) & 1UL;
}

void setBitToUL(uint32_t& number, byte bitnr, bool value) {
  uint32_t newbit = value ? 1UL : 0UL;
  number ^= (-newbit ^ number) & (1UL << bitnr);
}