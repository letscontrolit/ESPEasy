/*
TinyGPS++ - a small GPS library for Arduino providing universal NMEA parsing
Based on work by and "distanceBetween" and "courseTo" courtesy of Maarten Lamers.
Suggestion to add satellites, courseTo(), and cardinal() by Matt Monson.
Location precision improvements suggested by Wayne Holder.
Copyright (C) 2008-2013 Mikal Hart
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __TinyGPSPlus_h
#define __TinyGPSPlus_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <limits.h>

#define _GPS_VERSION "1.0.2" // software version of this library
#define _GPS_MPH_PER_KNOT 1.15077945
#define _GPS_MPS_PER_KNOT 0.51444444
#define _GPS_KMPH_PER_KNOT 1.852
#define _GPS_MILES_PER_METER 0.00062137112
#define _GPS_KM_PER_METER 0.001
#define _GPS_FEET_PER_METER 3.2808399
#define _GPS_MAX_FIELD_SIZE 15
#define _GPS_MAX_NR_ACTIVE_SATELLITES 16  // NMEA allows upto 12, some receivers report more using GSV strings
//#define _GPS_MAX_NR_SYSTEMS  3   // GPS, GLONASS, GALILEO
#define _GPS_MAX_NR_SYSTEMS  2   // GPS, GLONASS
#define _GPS_MAX_ARRAY_LENGTH  (_GPS_MAX_NR_ACTIVE_SATELLITES * _GPS_MAX_NR_SYSTEMS)

struct RawDegrees
{
   uint16_t deg;
   uint32_t billionths;
   bool negative;
public:
   RawDegrees() : deg(0), billionths(0), negative(false)
   {}
};

enum FixQuality { Invalid = 0, GPS = 1, DGPS = 2, PPS = 3, RTK = 4, FloatRTK = 5, Estimated = 6, Manual = 7, Simulated = 8 };
enum FixMode {
  N = 'N', // None
  A = 'A', // Autonomous
  D = 'D', // Differential
  E = 'E'};// Dead Reckoning

struct TinyGPSLocation
{
   friend class TinyGPSPlus;
public:
   bool isValid() const    { return valid; }
   bool isUpdated() const  { return updated; }
   uint32_t age() const    { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }
   const RawDegrees &rawLat()     { updated = false; return rawLatData; }
   const RawDegrees &rawLng()     { updated = false; return rawLngData; }
   double lat();
   double lng();
   FixQuality Quality() { /* updated = false; */ return fixQuality; }
   FixMode Mode() { /* updated = false; */ return fixMode; }

   TinyGPSLocation() : valid(false), updated(false), lastCommitTime(0), fixQuality(Invalid), newFixQuality(Invalid), fixMode(N), newFixMode(N)
   {}

private:
   bool valid, updated;
   RawDegrees rawLatData, rawLngData, rawNewLatData, rawNewLngData;
   uint32_t lastCommitTime;
   void commit();
   void setLatitude(const char *term);
   void setLongitude(const char *term);
   FixQuality fixQuality, newFixQuality;
   FixMode fixMode, newFixMode;
};

struct TinyGPSSatellites
{
  friend class TinyGPSPlus;
public:
  bool isValid() const       { return valid; }
  bool isUpdated() const     { return updated; }
  uint32_t age() const       { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }
  uint8_t nrSatsTracked() const { return satsTracked; }
  uint8_t nrSatsVisible() const { return satsVisible; }
  uint8_t getBestSNR() const { return bestSNR; }

  TinyGPSSatellites() : valid(false), updated(false), pos(-1), bestSNR(0), satsTracked(0), satsVisible(0), snrDataPresent(false), lastCommitTime(0)
  {}

  uint8_t id[_GPS_MAX_ARRAY_LENGTH] = {0};
  uint8_t snr[_GPS_MAX_ARRAY_LENGTH] = {0};

private:
   bool valid, updated;
   /*
   Satellite IDs:
    - 01 ~ 32 are for GPS
    - 33 ~ 64 are for SBAS (PRN minus 87)
    - 65 ~ 96 are for GLONASS (64 plus slot numbers)
    - 193 ~ 197 are for QZSS
    - 01 ~ 37 are for Beidou (BD PRN).
   GPS and Beidou satellites are differentiated by the GP and BD prefix.
   */
   int8_t pos; // Use signed int, only increase pos to set satellite ID
   uint8_t bestSNR;
   uint8_t satsTracked;
   uint8_t satsVisible;
   bool snrDataPresent;
   uint32_t lastCommitTime;

   void commit();
   void setSatId(const char *term);
   void setSatSNR(const char *term);

   // GSV messages form a sequence, so the initial position in the array must
   // be set before inserting values.
   void setMessageSeqNr(const char *term, uint8_t sentenceSystem);
};

struct TinyGPSDate
{
   friend class TinyGPSPlus;
public:
   bool isValid() const       { return valid; }
   bool isUpdated() const     { return updated; }
   uint32_t age() const       { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }

   uint32_t value()           { updated = false; return date; }
   uint16_t year();
   uint8_t month();
   uint8_t day();

   TinyGPSDate() : valid(false), updated(false), date(0), newDate(0), lastCommitTime(0)
   {}

private:
   bool valid, updated;
   uint32_t date, newDate;
   uint32_t lastCommitTime;
   void commit();
   void setDate(const char *term);
};

struct TinyGPSTime
{
   friend class TinyGPSPlus;
public:
   bool isValid() const       { return valid; }
   bool isUpdated() const     { return updated; }
   uint32_t age() const       { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }

   uint32_t value()           { updated = false; return time; }
   uint8_t hour();
   uint8_t minute();
   uint8_t second();
   uint8_t centisecond();

   TinyGPSTime() : valid(false), updated(false), time(0), newTime(0), lastCommitTime(0)
   {}

private:
   bool valid, updated;
   uint32_t time, newTime;
   uint32_t lastCommitTime;
   void commit();
   void setTime(const char *term);
};

struct TinyGPSDecimal
{
   friend class TinyGPSPlus;
public:
   bool isValid() const    { return valid; }
   bool isUpdated() const  { return updated; }
   uint32_t age() const    { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }
   int32_t value()         { updated = false; return val; }

   TinyGPSDecimal() : valid(false), updated(false), lastCommitTime(0), val(0), newval(0)
   {}

private:
   bool valid, updated;
   uint32_t lastCommitTime;
   int32_t val, newval;
   void commit();
   void set(const char *term);
};

struct TinyGPSInteger
{
   friend class TinyGPSPlus;
public:
   bool isValid() const    { return valid; }
   bool isUpdated() const  { return updated; }
   uint32_t age() const    { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }
   uint32_t value()        { updated = false; return val; }

   TinyGPSInteger() : valid(false), updated(false), lastCommitTime(0), val(0), newval(0)
   {}

private:
   bool valid, updated;
   uint32_t lastCommitTime;
   uint32_t val, newval;
   void commit();
   void set(const char *term);
};

struct TinyGPSSpeed : TinyGPSDecimal
{
   double knots()    { return value() / 100.0; }
   double mph()      { return _GPS_MPH_PER_KNOT * value() / 100.0; }
   double mps()      { return _GPS_MPS_PER_KNOT * value() / 100.0; }
   double kmph()     { return _GPS_KMPH_PER_KNOT * value() / 100.0; }
};

struct TinyGPSCourse : public TinyGPSDecimal
{
   double deg()      { return value() / 100.0; }
};

struct TinyGPSAltitude : TinyGPSDecimal
{
   double meters()       { return value() / 100.0; }
   double miles()        { return _GPS_MILES_PER_METER * value() / 100.0; }
   double kilometers()   { return _GPS_KM_PER_METER * value() / 100.0; }
   double feet()         { return _GPS_FEET_PER_METER * value() / 100.0; }
};

struct TinyGPSHDOP : TinyGPSDecimal
{
   double hdop() { return value() / 100.0; }
};

class TinyGPSPlus;
class TinyGPSCustom
{
public:
   TinyGPSCustom() {};
   TinyGPSCustom(TinyGPSPlus &gps, const char *sentenceName, int termNumber);
   void begin(TinyGPSPlus &gps, const char *_sentenceName, int _termNumber);

   bool isUpdated() const  { return updated; }
   bool isValid() const    { return valid; }
   uint32_t age() const    { return valid ? millis() - lastCommitTime : (uint32_t)ULONG_MAX; }
   const char *value()     { updated = false; return buffer; }

private:
   void commit();
   void set(const char *term);

   char stagingBuffer[_GPS_MAX_FIELD_SIZE + 1] = {0};
   char buffer[_GPS_MAX_FIELD_SIZE + 1] = {0};
   unsigned long lastCommitTime = 0;
   bool valid, updated = false;
   const char *sentenceName = nullptr;
   int termNumber = 0;
   friend class TinyGPSPlus;
   TinyGPSCustom *next = nullptr;
};

class TinyGPSPlus
{
public:
  TinyGPSPlus();
  bool encode(char c); // process one character received from GPS
  TinyGPSPlus &operator << (char c) {encode(c); return *this;}

  TinyGPSLocation location;
  TinyGPSDate date;
  TinyGPSTime time;
  TinyGPSSpeed speed;
  TinyGPSCourse course;
  TinyGPSAltitude altitude;
  TinyGPSInteger satellites;
  TinyGPSHDOP hdop;
  TinyGPSSatellites satellitesStats;

  static const char *libraryVersion() { return _GPS_VERSION; }

  static double distanceBetween(double lat1, double long1, double lat2, double long2);
  static double courseTo(double lat1, double long1, double lat2, double long2);
  static const char *cardinal(double course);

  static int32_t parseDecimal(const char *term);
  static void parseDegrees(const char *term, RawDegrees &deg);

  uint32_t charsProcessed()   const { return encodedCharCount; }
  uint32_t sentencesWithFix() const { return sentencesWithFixCount; }
  uint32_t failedChecksum()   const { return failedChecksumCount; }
  uint32_t passedChecksum()   const { return passedChecksumCount; }
  uint32_t invalidData()      const { return invalidDataCount; }

private:
  enum {
    GPS_SENTENCE_GPGGA,  // GGA - Global Positioning System Fix Data
    GPS_SENTENCE_GPRMC,  // RMC - Recommended Minimum Navigation Information

    GPS_SENTENCE_GPGSA,  // GSA - GPS DOP and active satellites
    GPS_SENTENCE_GPGSV,  // GSV - Satellites in view

    GPS_SENTENCE_OTHER};

  enum {
    GPS_SYSTEM_GPS = 0, // GP: GPS, SBAS, QZSS  & GN: Any combination of GNSS
    GPS_SYSTEM_GLONASS,
    GPS_SYSTEM_GALILEO,
    GPS_SYSTEM_BEIDOU
  };

  void parseSentenceType(const char *term);

  // parsing state variables
  uint8_t parity = 0;
  bool isChecksumTerm = false;
  char term[_GPS_MAX_FIELD_SIZE] = {0};
  uint8_t curSentenceType = 0;
  uint8_t curSentenceSystem = 0;
  uint8_t curTermNumber = 0;
  uint8_t curTermOffset = 0;
  bool sentenceHasFix = false;

  // custom element support
  friend class TinyGPSCustom;
  TinyGPSCustom *customElts = nullptr;
  TinyGPSCustom *customCandidates = nullptr;
  void insertCustom(TinyGPSCustom *pElt, const char *sentenceName, int index);

  // statistics
  uint32_t encodedCharCount = 0;
  uint32_t sentencesWithFixCount = 0;
  uint32_t failedChecksumCount = 0;
  uint32_t passedChecksumCount = 0;
  uint32_t invalidDataCount = 0;

  // internal utilities
  int fromHex(char a);
  bool endOfTermHandler();
};

#endif // def(__TinyGPSPlus_h)
