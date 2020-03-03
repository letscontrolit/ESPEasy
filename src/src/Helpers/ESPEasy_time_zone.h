#ifndef HELPERS_ESPEASY_TIME_ZONE_H
#define HELPERS_ESPEASY_TIME_ZONE_H

#include <Arduino.h>

#include "../DataStructs/TimeChangeRule.h"

/********************************************************************************************\
   Time zone
 \*********************************************************************************************/

// Borrowed code from Timezone: https://github.com/JChristensen/Timezon



/*
   // Examples time zones
   // Australia Eastern Time Zone (Sydney, Melbourne)
   TimeChangeRule aEDT = {First, Sun, Oct, 2, 660};    // UTC + 11 hours
   TimeChangeRule aEST = {First, Sun, Apr, 3, 600};    // UTC + 10 hours
   setTimeZone(aEDT, aEST);

   // Central European Time (Frankfurt, Paris)
   TimeChangeRule CEST = {Last, Sun, Mar, 2, 120};     // Central European Summer Time
   TimeChangeRule CET = {Last, Sun, Oct, 3, 60};       // Central European Standard Time
   setTimeZone(CEST, CET);

   // United Kingdom (London, Belfast)
   TimeChangeRule BST = {Last, Sun, Mar, 1, 60};        // British Summer Time
   TimeChangeRule GMT = {Last, Sun, Oct, 2, 0};         // Standard Time
   setTimeZone(BST, GMT);

   // UTC
   TimeChangeRule utcRule = {Last, Sun, Mar, 1, 0};     // UTC
   setTimeZone(utcRule, utcRule);

   // US Eastern Time Zone (New York, Detroit)
   TimeChangeRule usEDT = {Second, Sun, Mar, 2, -240};  // Eastern Daylight Time = UTC - 4 hours
   TimeChangeRule usEST = {First, Sun, Nov, 2, -300};   // Eastern Standard Time = UTC - 5 hours
   setTimeZone(usEDT, usEST);

   // US Central Time Zone (Chicago, Houston)
   TimeChangeRule usCDT = {Second, dowSunday, Mar, 2, -300};
   TimeChangeRule usCST = {First, dowSunday, Nov, 2, -360};
   setTimeZone(usCDT, usCST);

   // US Mountain Time Zone (Denver, Salt Lake City)
   TimeChangeRule usMDT = {Second, dowSunday, Mar, 2, -360};
   TimeChangeRule usMST = {First, dowSunday, Nov, 2, -420};
   setTimeZone(usMDT, usMST);

   // Arizona is US Mountain Time Zone but does not use DST
   setTimeZone(usMST, usMST);

   // US Pacific Time Zone (Las Vegas, Los Angeles)
   TimeChangeRule usPDT = {Second, dowSunday, Mar, 2, -420};
   TimeChangeRule usPST = {First, dowSunday, Nov, 2, -480};
   setTimeZone(usPDT, usPST);
 */


class ESPEasy_time_zone {

public:


void getDefaultDst_flash_values(uint16_t& start, uint16_t& end);

void applyTimeZone(uint32_t curTime);

void setTimeZone(const TimeChangeRule& dstStart, const TimeChangeRule& stdStart, uint32_t curTime);

void logTimeZoneInfo();


///*----------------------------------------------------------------------*
// * Convert the given time change rule to a uint32_t value               *
// * for the given year.                                                  *
// *----------------------------------------------------------------------*/
uint32_t calcTimeChangeForRule(const TimeChangeRule& r, int yr);

/*----------------------------------------------------------------------*
* Calculate the DST and standard time change points for the given       *
* given year as local and UTC uint32_t values.                          *
*----------------------------------------------------------------------*/
bool calcTimeChanges(int yr);

/*----------------------------------------------------------------------*
* Convert the given UTC time to local time, standard or                 *
* daylight time, as appropriate.                                        *
*-----------------------------------------------------------------------*/
uint32_t toLocal(uint32_t utc);

/*----------------------------------------------------------------------*
* Determine whether the given UTC uint32_t is within the DST interval   *
* or the Standard time interval.                                        *
*-----------------------------------------------------------------------*/
bool utcIsDST(uint32_t utc);

/*----------------------------------------------------------------------*
* Determine whether the given Local uint32_t is within the DST interval *
* or the Standard time interval.                                        *
*-----------------------------------------------------------------------*/
bool locIsDST(uint32_t local);


TimeChangeRule m_dst;  // rule for start of dst or summer time for any year
TimeChangeRule m_std;  // rule for start of standard time for any year
uint32_t m_dstUTC = 0; // dst start for given/current year, given in UTC
uint32_t m_stdUTC = 0; // std time start for given/current year, given in UTC
uint32_t m_dstLoc = 0; // dst start for given/current year, given in local time
uint32_t m_stdLoc = 0; // std time start for given/current year, given in local time


};


#endif // HELPERS_ESPEASY_TIME_ZONE_H