/*********************************************************************************************\
   Get value count from sensor type
  \*********************************************************************************************/

byte getValueCountFromSensorType(byte sensorType)
{
  switch (sensorType)
  {
    case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
    case SENSOR_TYPE_SWITCH:
    case SENSOR_TYPE_DIMMER:
      return 1;
    case SENSOR_TYPE_LONG:                      // single LONG value, stored in two floats (rfid tags)
      return 1;
    case SENSOR_TYPE_TEMP_HUM:
    case SENSOR_TYPE_TEMP_BARO:
    case SENSOR_TYPE_DUAL:
      return 2;
    case SENSOR_TYPE_TEMP_HUM_BARO:
    case SENSOR_TYPE_TRIPLE:
    case SENSOR_TYPE_WIND:
      return 3;
    case SENSOR_TYPE_QUAD:
      return 4;
  }
  addLog(LOG_LEVEL_ERROR, F("getValueCountFromSensorType: Unknown sensortype"));
  return 0;
}

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
