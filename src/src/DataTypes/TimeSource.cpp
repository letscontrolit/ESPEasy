#include "../DataTypes/TimeSource.h"

const __FlashStringHelper* toString(ExtTimeSource_e timeSource)
{
  switch (timeSource) {
    case ExtTimeSource_e::None: break;
    case ExtTimeSource_e::DS1307:  return F("DS1307");
    case ExtTimeSource_e::DS3231:  return F("DS3231");
    case ExtTimeSource_e::PCF8523: return F("PCF8523");
    case ExtTimeSource_e::PCF8563: return F("PCF8563");
  }
  return F("-");
}
