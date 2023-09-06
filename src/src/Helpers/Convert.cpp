#include "../Helpers/Convert.h"

#include "../Helpers/StringConverter.h"

/*********************************************************************************************\
   Convert bearing in degree to bearing string
\*********************************************************************************************/
const __FlashStringHelper * getBearing(int degrees)
{
  const __FlashStringHelper* directions[] {
    F("N"),
    F("NNE"),
    F("NE"),
    F("ENE"),
    F("E"),
    F("ESE"),
    F("SE"),
    F("SSE"),
    F("S"),
    F("SSW"),
    F("SW"),
    F("WSW"),
    F("W"),
    F("WNW"),
    F("NW"),
    F("NNW")
  };
  constexpr size_t nrDirections = NR_ELEMENTS(directions);
  const float stepsize          = (360.0f / nrDirections);

  if (degrees < 0) { degrees += 360; } // Allow for bearing -360 .. 359
  const size_t bearing_idx = int((degrees + (stepsize / 2.0f)) / stepsize) % nrDirections;

  if (bearing_idx < nrDirections) {
    return directions[bearing_idx];
  }
  return F("");
}

float CelsiusToFahrenheit(float celsius) {
  constexpr float ratio = 9.0f / 5.0f;
  return celsius * ratio + 32;
}

int m_secToBeaufort(float m_per_sec) {
  // Use ints wit 0.1 m/sec resolution to reduce size.
  const uint16_t dm_per_sec = 10 * m_per_sec;
  const uint16_t speeds[]{3, 16, 34, 55, 80, 108, 139, 172, 208, 245, 285, 326};  
  constexpr int nrElements = NR_ELEMENTS(speeds);
  
  for (int bft = 0; bft < nrElements; ++bft) {
    if (dm_per_sec < speeds[bft]) return bft;
  }
  return nrElements;  
}

String centimeterToImperialLength(float cm) {
  return millimeterToImperialLength(cm * 10.0f);
}

String millimeterToImperialLength(float mm) {
  float inches = mm / 25.4f;
  int   feet   = inches / 12.0f;

  inches = inches - (feet * 12);
  String result;
  result.reserve(10);

  if (feet != 0) {
    result += feet;
    result += '\'';
  }
  result += toString(inches, 1);
  result += '"';
  return result;
}

float minutesToDay(int minutes) {
  return minutes / 1440.0f;
}

String minutesToDayHour(int minutes) {
  int  days  = minutes / 1440;
  int  hours = (minutes % 1440) / 60;
  char TimeString[8] = {0}; // 5 digits plus the null char minimum

  sprintf_P(TimeString, PSTR("%d%c%02d%c"), days, 'd', hours, 'h');
  return TimeString;
}

String minutesToHourMinute(int minutes) {
  int  hours = (minutes % 1440) / 60;
  int  mins  = (minutes % 1440) % 60;
  char TimeString[20] = {0};

  sprintf_P(TimeString, PSTR("%d%c%02d%c"), hours, 'h', mins, 'm');
  return TimeString;
}

String minutesToDayHourMinute(int minutes) {
  int  days  = minutes / 1440;
  int  hours = (minutes % 1440) / 60;
  int  mins  = (minutes % 1440) % 60;
  char TimeString[20] = {0};

  sprintf_P(TimeString, PSTR("%d%c%02d%c%02d%c"), days, 'd', hours, 'h', mins, 'm');
  return TimeString;
}

String minutesToHourColonMinute(int minutes) {
  int  hours = (minutes % 1440) / 60;
  int  mins  = (minutes % 1440) % 60;
  char TimeString[8] = {0};

  sprintf_P(TimeString, PSTR("%02d%c%02d"), hours, ':', mins);
  return TimeString;
}

String secondsToDayHourMinuteSecond(int seconds) {
  int  sec     = seconds % 60;
  int  minutes = seconds / 60;
  int  days    = minutes / 1440;
  int  hours   = (minutes % 1440) / 60;
  int  mins    = (minutes % 1440) % 60;
  char TimeString[20] = {0};

  sprintf_P(TimeString, PSTR("%d%c%02d%c%02d%c%02d"), days, 'd', hours, ':', mins, ':', sec);
  return TimeString;
}

String format_msec_duration(int64_t duration) {
  String result;

  if (duration < 0) {
    result   = '-';
    duration = -1ll * duration;
  }

  if (duration < 10000ll) {
    result += static_cast<int32_t>(duration);
    result += F(" ms");
    return result;
  }
  duration /= 1000ll;

  if (duration < 3600ll) {
    int sec     = duration % 60ll;
    int minutes = duration / 60ll;

    if (minutes > 0ll) {
      result += minutes;
      result += F(" m ");
    }
    result += sec;
    result += F(" s");
    return result;
  }
  duration /= 60ll;

  if (duration < 1440ll) { return minutesToHourMinute(duration); }
  return minutesToDayHourMinute(duration);
}


// Compute the dew point temperature, given temperature and humidity (temp in Celsius)
// Formula: http://www.ajdesigner.com/phphumidity/dewpoint_equation_dewpoint_temperature.php
// Td = (f/100)^(1/8) * (112 + 0.9*T) + 0.1*T - 112
float compute_dew_point_temp(float temperature, float humidity_percentage) {
  return powf(humidity_percentage / 100.0f, 0.125f) *
         (112.0f + 0.9f*temperature) + 0.1f*temperature - 112.0f;
}

// Compute the humidity given temperature and dew point temperature (temp in Celsius)
// Formula: http://www.ajdesigner.com/phphumidity/dewpoint_equation_relative_humidity.php
// f = 100 * ((112 - 0.1*T + Td) / (112 + 0.9 * T))^8
float compute_humidity_from_dewpoint(float temperature, float dew_temperature) {
  return 100.0f * powf((112.0f - 0.1f * temperature + dew_temperature) /
                     (112.0f + 0.9f * temperature), 8);
}



/********************************************************************************************\
   Compensate air pressure for given altitude (in meters)
 \*********************************************************************************************/
float pressureElevation(float atmospheric, float altitude) {
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064
  return atmospheric / powf(1.0f - (altitude / 44330.0f), 5.255f);
}

float altitudeFromPressure(float atmospheric, float seaLevel)
{
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064
  return 44330.0f * (1.0f - powf(atmospheric / seaLevel, 0.1903f));
}




/********************************************************************************************\
   In memory convert float to long
 \*********************************************************************************************/
unsigned long float2ul(float f)
{
  unsigned long ul;

  memcpy(&ul, &f, 4);
  return ul;
}

/********************************************************************************************\
   In memory convert long to float
 \*********************************************************************************************/
float ul2float(unsigned long ul)
{
  float f;

  memcpy(&f, &ul, 4);
  return f;
}


