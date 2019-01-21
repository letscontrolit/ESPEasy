/*********************************************************************************************\
   Get value count from sensor type
  \*********************************************************************************************/

byte getValueCountFromSensorType(byte sensorType)
{
  if (sensorType > 100)
    return sensorType-100;

  switch (sensorType)
  {
    case SENSOR_TYPE_NONE:
      return 0;
    case SENSOR_TYPE_SINGLE: // single value sensor, used for Dallas, BH1750, etc
    case SENSOR_TYPE_SWITCH:
    case SENSOR_TYPE_DIMMER:
      return 1;
    case SENSOR_TYPE_LONG:   // single LONG value, stored in two floats (rfid tags)
      return 1;
    case SENSOR_TYPE_TEMP_HUM:
    case SENSOR_TYPE_TEMP_BARO:
    case SENSOR_TYPE_DUAL:
      return 2;
    case SENSOR_TYPE_TEMP_HUM_BARO:
    case SENSOR_TYPE_TEMP_EMPTY_BARO: // Values 1 and 3 will contain data.
    case SENSOR_TYPE_TRIPLE:
    case SENSOR_TYPE_WIND:
      return 3;
    case SENSOR_TYPE_QUAD:
      return 4;
    case SENSOR_TYPE_PENTA:
      return 5;
  }
  addLog(LOG_LEVEL_ERROR, F("getValueCountFromSensorType: Unknown sensortype"));
  return 0;
}
