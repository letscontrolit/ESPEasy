#ifndef DATATYPES_TIMESOURCE_H
#define DATATYPES_TIMESOURCE_H

#include <Arduino.h>

// Do not change order as values are stored in settings
enum class ExtTimeSource_e {
  None = 0,
  DS1307,
  DS3231,
  PCF8523,
  PCF8563
};


const __FlashStringHelper* toString(ExtTimeSource_e timeSource);


#endif // ifndef DATATYPES_TIMESOURCE_H
