#include "Convert.h"

/*********************************************************************************************\
   Convert bearing in degree to bearing string
\*********************************************************************************************/
String getBearing(int degrees)
{
  const int nr_directions = 16;
  float stepsize      = (360.0f / nr_directions);

  if (degrees < 0) { degrees += 360; } // Allow for bearing -360 .. 359
  int bearing_idx = int((degrees + (stepsize / 2.0f)) / stepsize) % nr_directions;

  if (bearing_idx >= 0) {
    switch (bearing_idx) {
      case 0: return F("N");
      case 1: return F("NNE");
      case 2: return F("NE");
      case 3: return F("ENE");
      case 4: return F("E");
      case 5: return F("ESE");
      case 6: return F("SE");
      case 7: return F("SSE");
      case 8: return F("S");
      case 9: return F("SSW");
      case 10: return F("SW");
      case 11: return F("WSW");
      case 12: return F("W");
      case 13: return F("WNW");
      case 14: return F("NW");
      case 15: return F("NNW");
    }
  }
  return "";
}

float CelsiusToFahrenheit(float celsius) {
  return celsius * (9.0f / 5.0f) + 32;
}

int m_secToBeaufort(float m_per_sec) {
  if (m_per_sec < 0.3f) { return 0; }

  if (m_per_sec < 1.6f) { return 1; }

  if (m_per_sec < 3.4f) { return 2; }

  if (m_per_sec < 5.5f) { return 3; }

  if (m_per_sec < 8.0f) { return 4; }

  if (m_per_sec < 10.8f) { return 5; }

  if (m_per_sec < 13.9f) { return 6; }

  if (m_per_sec < 17.2f) { return 7; }

  if (m_per_sec < 20.8f) { return 8; }

  if (m_per_sec < 24.5f) { return 9; }

  if (m_per_sec < 28.5f) { return 10; }

  if (m_per_sec < 32.6f) { return 11; }
  return 12;
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
    result   = "-";
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
  return pow(humidity_percentage / 100.0f, 0.125f) *
         (112.0f + 0.9f*temperature) + 0.1f*temperature - 112.0f;
}

// Compute the humidity given temperature and dew point temperature (temp in Celsius)
// Formula: http://www.ajdesigner.com/phphumidity/dewpoint_equation_relative_humidity.php
// f = 100 * ((112 - 0.1*T + Td) / (112 + 0.9 * T))^8
float compute_humidity_from_dewpoint(float temperature, float dew_temperature) {
  return 100.0f * pow((112.0f - 0.1f * temperature + dew_temperature) /
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
  return atmospheric / pow(1.0f - (altitude / 44330.0f), 5.255f);
}

float altitudeFromPressure(float atmospheric, float seaLevel)
{
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064
  return 44330.0f * (1.0f - pow(atmospheric / seaLevel, 0.1903f));
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

/*********************************************************************************************\
   Workaround for removing trailing white space when String() converts a float with 0 decimals
\*********************************************************************************************/
String toString(const float& value, byte decimals)
{
  String sValue = String(value, decimals);

  sValue.trim();
  return sValue;
}

String doubleToString(const double& value, int decimals, bool trimTrailingZeros) {
  String res(value, decimals);
  if (trimTrailingZeros) {
    int dot_pos = res.lastIndexOf('.');
    if (dot_pos != -1) {
      bool someTrimmed = false;
      for (int i = res.length()-1; i > dot_pos && res[i] == '0'; --i) {
        someTrimmed = true;
        res[i] = ' ';
      }
      if (someTrimmed) {
        res.trim();
      }
      if (res.endsWith(F("."))) {
        res[dot_pos] = ' ';
        res.trim();
      }
    }
  }
  return res;
}
