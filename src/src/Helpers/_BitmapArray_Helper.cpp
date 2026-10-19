#include "../Helpers/_BitmapArray_Helper.h"

/**
 * Gather the Index or Count for the _id by skipping inbetween 0-bits, no validity checks here.
 * Also determines the highest available ID + 1 for sizing an array
 * Will return the Count if _id == _max!
 * Using uint8_t for _id and return value as that's the basetype for pluginID_t and cpluginID_t,
 * and also because the max. usable value is 8 bits (without major surgery on ESPEasy core)
 */
uint8_t getIndexFromBitmap(const uint16_t _bitmap[], uint8_t _id, uint8_t _max, size_t& _highest) {
  uint8_t count = 0;
  uint8_t index = 0;

  // Account for IDs to be 1-based
  for (uint8_t n = 1; n < _max && count < _id + 1; n++) {
    const uint16_t wordIndex = (n - 1) / 16;
    const uint16_t bitIndex  = 15 - ((n - 1) % 16);

    if (bitRead(_bitmap[wordIndex], bitIndex)) {
      count++;          // Count
      index    = n;
      _highest = n + 1; // Highest ID + 1
    }
  }
  return _id == _max ? count : index;
}

/**
 * Gather the ID for the _index by skipping inbetween 0-bits, no validity checks here.
 * Using uint8_t for _index and return value as that's the basetype for pluginID_t and cpluginID_t,
 * and also because the max. usable value is 8 bits (without major surgery on ESPEasy core)
 */
uint8_t getIdFromBitmap(const uint16_t _bitmap[], uint8_t _index, uint8_t _max, uint8_t _invalid) {
  uint8_t result = 0;
  uint8_t id     = _invalid;

  // Account for IDs to be 1-based
  for (uint8_t n = 1; n < _max && result < _index; n++) {
    const uint16_t wordIndex = (n - 1) / 16;
    const uint16_t bitIndex  = 15 - ((n - 1) % 16);

    if (bitRead(_bitmap[wordIndex], bitIndex)) {
      result++; // Count
      id = n;   // ID
    }
  }
  return id;
}
