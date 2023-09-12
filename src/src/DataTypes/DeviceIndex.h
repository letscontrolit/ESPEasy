#ifndef DATATYPES_DEVICEINDEX_H
#define DATATYPES_DEVICEINDEX_H

#include "../../ESPEasy_common.h"

// typedef uint8_t deviceIndex_t;

struct deviceIndex_t {
  deviceIndex_t() = default;

  static deviceIndex_t toDeviceIndex(int other);

  // TD-er: Do not add constructor with int as argument,
  // as this makes it impossible for the compiler to typecheck its use.
  //  deviceIndex_t(int other);

  deviceIndex_t& operator=(int other);


  deviceIndex_t& operator=(const deviceIndex_t& other);

  bool           operator<(int other) const;
  bool           operator<=(int other) const;
  bool           operator!=(int other) const;
  bool           operator!=(const deviceIndex_t& other) const;

  deviceIndex_t& operator++();

//  bool           isValid() const;


  // TD-er: Do not add operator int() as it makes it impossible for the compiler to typecheck use of this struct.
  /*
     operator int() const
     {
      return value;
     }
   */

  uint8_t value{}; // Init this to 0, so we can easily iterate over it.
};


extern deviceIndex_t INVALID_DEVICE_INDEX;

#endif // ifndef DATATYPES_DEVICEINDEX_H
