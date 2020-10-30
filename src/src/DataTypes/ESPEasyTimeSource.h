#ifndef ESPEASY_TIMETYPES_H_
#define ESPEASY_TIMETYPES_H_

#include <stdint.h>
#include <list>
#include <time.h>

#include "../../src/DataStructs/TimeChangeRule.h"
#include "../../src/Globals/Plugins.h"


enum timeSource_t {
  No_time_source,
  NTP_time_source,
  Restore_RTC_time_source,
  GPS_time_source,
  Manual_set
};

#endif /* ESPEASY_TIMETYPES_H_ */
