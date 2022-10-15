#ifndef HELPERS_CONVERT_H
#define HELPERS_CONVERT_H

#include <Arduino.h>

/*********************************************************************************************\
   Convert bearing in degree to bearing string
\*********************************************************************************************/
const __FlashStringHelper * getBearing(int degrees);

float CelsiusToFahrenheit(float celsius);

int m_secToBeaufort(float m_per_sec);

String centimeterToImperialLength(float cm);

String millimeterToImperialLength(float mm);

float minutesToDay(int minutes);

String minutesToDayHour(int minutes);

String minutesToHourMinute(int minutes);

String minutesToDayHourMinute(int minutes);

String minutesToHourColonMinute(int minutes);

String secondsToDayHourMinuteSecond(int seconds);

String format_msec_duration(int64_t duration);

// Compute the dew point temperature, given temperature and humidity (temp in Celsius)
// Formula: http://www.ajdesigner.com/phphumidity/dewpoint_equation_dewpoint_temperature.php
// Td = (f/100)^(1/8) * (112 + 0.9*T) + 0.1*T - 112
float compute_dew_point_temp(float temperature, float humidity_percentage);

// Compute the humidity given temperature and dew point temperature (temp in Celsius)
// Formula: http://www.ajdesigner.com/phphumidity/dewpoint_equation_relative_humidity.php
// f = 100 * ((112 - 0.1*T + Td) / (112 + 0.9 * T))^8
float compute_humidity_from_dewpoint(float temperature, float dew_temperature);

/********************************************************************************************\
   Compensate air pressure for measured atmospheric
      pressure (in hPa) and given altitude (in meters)
 \*********************************************************************************************/
float pressureElevation(float atmospheric, float altitude);

/********************************************************************************************\
  Calculates the altitude (in meters) from the specified atmospheric
      pressure (in hPa), and sea-level pressure (in hPa).
      @param  seaLevel      Sea-level pressure in hPa
      @param  atmospheric   Atmospheric pressure in hPa
 \*********************************************************************************************/
float altitudeFromPressure(float atmospheric, float seaLevel);

/********************************************************************************************\
   In memory convert float to long
 \*********************************************************************************************/
unsigned long float2ul(float f);

/********************************************************************************************\
   In memory convert long to float
 \*******************************************************************************************/
float ul2float(unsigned long ul);

/*********************************************************************************************\
   Workaround for removing trailing white space when String() converts a float with 0 decimals
\*********************************************************************************************/
String toString(const float& value, unsigned int decimalPlaces = 2);

String doubleToString(const double& value, unsigned int decimalPlaces = 2, bool trimTrailingZeros = false);


#endif // HELPERS_CONVERT_H