#ifndef ESPEASY_TIMETYPES_H_
#define ESPEASY_TIMETYPES_H_

#include <stdint.h>
#include <list>
#include <time.h>

#include "src/DataStructs/TimeChangeRule.h"
#include "src/Globals/Plugins.h"


enum timeSource_t {
  No_time_source,
  NTP_time_source,
  Restore_RTC_time_source,
  GPS_time_source,
  Manual_set
};


// Forward declartions

void     setTimeZone(const TimeChangeRule& dstStart,
                     const TimeChangeRule& stdStart,
                     uint32_t              curTime = 0);
uint32_t calcTimeChangeForRule(const TimeChangeRule& r,
                               int                   yr);
String   getTimeString(char delimiter,
                       bool show_seconds = true);
String   getTimeString_ampm(char delimiter,
                            bool show_seconds = true);

void     setPluginTaskTimer(unsigned long msecFromNow,
                            taskIndex_t   taskIndex,
                            int           Par1,
                            int           Par2 = 0,
                            int           Par3 = 0,
                            int           Par4 = 0,
                            int           Par5 = 0);
void     setPluginTimer(unsigned long msecFromNow,
                        pluginID_t   pluginID,
                        int           Par1,
                        int           Par2 = 0,
                        int           Par3 = 0,
                        int           Par4 = 0,
                        int           Par5 = 0);
void setGPIOTimer(unsigned long msecFromNow,
                  int           Par1,
                  int           Par2 = 0,
                  int           Par3 = 0,
                  int           Par4 = 0,
                  int           Par5 = 0);




#endif /* ESPEASY_TIMETYPES_H_ */
