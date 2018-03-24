
/*********************************************************************************************\
   Convert bearing in degree to bearing string
  \*********************************************************************************************/

String getBearing(int degrees)
{
  const __FlashStringHelper* bearing[] = {
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
  int nr_directions = (int) (sizeof(bearing)/sizeof(bearing[0]));
  float stepsize = (360.0 / nr_directions);
  if (degrees < 0) { degrees += 360; } // Allow for bearing -360 .. 359
  int bearing_idx=int((degrees + (stepsize / 2.0)) / stepsize) % nr_directions;
  if (bearing_idx < 0)
    return("");
  else
    return(bearing[bearing_idx]);
}

float CelsiusToFahrenheit(float celsius) {
  return celsius * (9.0 / 5.0) + 32;
}

int m_secToBeaufort(float m_per_sec) {
  if (m_per_sec < 0.3) return 0;
  if (m_per_sec < 1.6) return 1;
  if (m_per_sec < 3.4) return 2;
  if (m_per_sec < 5.5) return 3;
  if (m_per_sec < 8.0) return 4;
  if (m_per_sec < 10.8) return 5;
  if (m_per_sec < 13.9) return 6;
  if (m_per_sec < 17.2) return 7;
  if (m_per_sec < 20.8) return 8;
  if (m_per_sec < 24.5) return 9;
  if (m_per_sec < 28.5) return 10;
  if (m_per_sec < 32.6) return 11;
  return 12;
}

String centimeterToImperialLength(float cm) {
  return millimeterToImperialLength(cm * 10.0);
}

String millimeterToImperialLength(float mm) {
  float inches = mm / 25.4;
  int feet = inches /12;
  inches = inches - (feet * 12);
  String result;
  result.reserve(10);
  if (feet != 0) {
    result += feet;
    result += '\'';
  }
  result += toString(inches,1);
  result += '"';
  return result;
}

float minutesToDay(int minutes) {
  return minutes / 1440.0;
}

String minutesToDayHour(int minutes) {
  int days = minutes / 1440;
  int hours = (minutes % 1440) / 60;
  char TimeString[6]; //5 digits plus the null char
  sprintf_P(TimeString, PSTR("%d%c%02d%c"), days, 'd', hours, 'h');
  return TimeString;
}

String minutesToHourMinute(int minutes) {
  int hours = (minutes % 1440) / 60;
  int mins = (minutes % 1440) % 60;
  char TimeString[20];
  sprintf_P(TimeString, PSTR("%d%c%02d%c"), hours, 'h', mins, 'm');
  return TimeString;
}

String minutesToDayHourMinute(int minutes) {
  int days = minutes / 1440;
  int hours = (minutes % 1440) / 60;
  int mins = (minutes % 1440) % 60;
  char TimeString[20];
  sprintf_P(TimeString, PSTR("%d%c%02d%c%02d%c"), days, 'd', hours, 'h', mins, 'm');
  return TimeString;
}

String secondsToDayHourMinuteSecond(int seconds) {
  int sec = seconds % 60;
  int minutes = seconds / 60;
  int days = minutes / 1440;
  int hours = (minutes % 1440) / 60;
  int mins = (minutes % 1440) % 60;
  char TimeString[20];
  sprintf_P(TimeString, PSTR("%d%c%02d%c%02d%c%02d"), days, 'd', hours, ':', mins, ':', sec);
  return TimeString;
}

String format_msec_duration(long duration) {
  String result;
  if (duration < 0) {
    result = "-";
    duration = -1 * duration;
  }
  if (duration < 10000) {
    result += duration;
    result += F(" ms");
    return result;
  }
  duration /= 1000;
  if (duration < 3600) {
    int sec = duration % 60;
    int minutes = duration / 60;
    if (minutes > 0) {
      result += minutes;
      result += F(" m ");
    }
    result += sec;
    result += F(" s");
    return result;
  }
  duration /= 60;
  if (duration < 1440) return minutesToHourMinute(duration);
  return minutesToDayHourMinute(duration);
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
