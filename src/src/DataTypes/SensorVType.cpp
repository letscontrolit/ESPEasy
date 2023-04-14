#include "../DataTypes/SensorVType.h"


/*********************************************************************************************\
   Get value count from sensor type

   Only use this function to determine nr of output values when changing output type of a task
   To get the actual output values for a task, use getValueCountForTask
\*********************************************************************************************/
uint8_t getValueCountFromSensorType(Sensor_VType sensorType)
{
  switch (sensorType)
  {
    case Sensor_VType::SENSOR_TYPE_NONE:
      return 0;
    case Sensor_VType::SENSOR_TYPE_SINGLE:        // single value sensor, used for Dallas, BH1750, etc
    case Sensor_VType::SENSOR_TYPE_ULONG_SINGLE:  // 1x uint32_t
    case Sensor_VType::SENSOR_TYPE_UINT64_SINGLE: // 1x uint64_t
    case Sensor_VType::SENSOR_TYPE_INT64_SINGLE:  // 1x int64_t
    case Sensor_VType::SENSOR_TYPE_DOUBLE_SINGLE: // 1x double
    case Sensor_VType::SENSOR_TYPE_SWITCH:
    case Sensor_VType::SENSOR_TYPE_DIMMER:
    case Sensor_VType::SENSOR_TYPE_LONG:          // single LONG value, stored in two floats (rfid tags)
    case Sensor_VType::SENSOR_TYPE_STRING:        // String type data stored in the event->String2
      return 1;
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM:
    case Sensor_VType::SENSOR_TYPE_TEMP_BARO:
    case Sensor_VType::SENSOR_TYPE_DUAL:
    case Sensor_VType::SENSOR_TYPE_ULONG_DUAL:      // 2x uint32_t
    case Sensor_VType::SENSOR_TYPE_LONG_DUAL:       // 2x int32_t
    case Sensor_VType::SENSOR_TYPE_UINT64_DUAL:     // 2x uint64_t
    case Sensor_VType::SENSOR_TYPE_INT64_DUAL:      // 2x int64_t
    case Sensor_VType::SENSOR_TYPE_DOUBLE_DUAL:     // 2x double
      return 2;
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO:
    case Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO: // Values 1 and 3 will contain data.
    case Sensor_VType::SENSOR_TYPE_TRIPLE:
    case Sensor_VType::SENSOR_TYPE_ULONG_TRIPLE:    // 3x uint32_t
    case Sensor_VType::SENSOR_TYPE_LONG_TRIPLE:     // 3x int32_t
    case Sensor_VType::SENSOR_TYPE_WIND:
      return 3;
    case Sensor_VType::SENSOR_TYPE_QUAD:
    case Sensor_VType::SENSOR_TYPE_ULONG_QUAD: // 4x uint32_t
    case Sensor_VType::SENSOR_TYPE_LONG_QUAD:  // 4x int32_t
      return 4;
    case Sensor_VType::SENSOR_TYPE_NOT_SET:  break;
  }
  #ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_ERROR, F("getValueCountFromSensorType: Unknown sensortype"));
  #endif // ifndef BUILD_NO_DEBUG
  return 0;
}

const __FlashStringHelper* getSensorTypeLabel(Sensor_VType sensorType) {
  switch (sensorType) {
    case Sensor_VType::SENSOR_TYPE_SINGLE:           return F("Single");
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM:         return F("Temp / Hum");
    case Sensor_VType::SENSOR_TYPE_TEMP_BARO:        return F("Temp / Baro");
    case Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO:  return F("Temp / - / Baro");
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO:    return F("Temp / Hum / Baro");
    case Sensor_VType::SENSOR_TYPE_DUAL:             return F("Dual");
    case Sensor_VType::SENSOR_TYPE_TRIPLE:           return F("Triple");
    case Sensor_VType::SENSOR_TYPE_QUAD:             return F("Quad");
    case Sensor_VType::SENSOR_TYPE_SWITCH:           return F("Switch");
    case Sensor_VType::SENSOR_TYPE_DIMMER:           return F("Dimmer");
    case Sensor_VType::SENSOR_TYPE_LONG:             return F("Long Single");
    case Sensor_VType::SENSOR_TYPE_LONG_DUAL:        return F("Long Dual");
    case Sensor_VType::SENSOR_TYPE_LONG_TRIPLE:      return F("Long Triple");
    case Sensor_VType::SENSOR_TYPE_LONG_QUAD:        return F("Long Quad");
    case Sensor_VType::SENSOR_TYPE_ULONG_SINGLE:     return F("ULong Single");
    case Sensor_VType::SENSOR_TYPE_ULONG_DUAL:       return F("ULong Dual");
    case Sensor_VType::SENSOR_TYPE_ULONG_TRIPLE:     return F("ULong Triple");
    case Sensor_VType::SENSOR_TYPE_ULONG_QUAD:       return F("ULong Quad");
    case Sensor_VType::SENSOR_TYPE_UINT64_SINGLE:    return F("UInt64 Single");
    case Sensor_VType::SENSOR_TYPE_UINT64_DUAL:      return F("UInt64 Dual");
    case Sensor_VType::SENSOR_TYPE_INT64_SINGLE:     return F("Int64 Single");
    case Sensor_VType::SENSOR_TYPE_INT64_DUAL:       return F("Int64 Dual");
    case Sensor_VType::SENSOR_TYPE_DOUBLE_SINGLE:    return F("Double Single");
    case Sensor_VType::SENSOR_TYPE_DOUBLE_DUAL:      return F("Double Dual");
    case Sensor_VType::SENSOR_TYPE_WIND:             return F("Wind");
    case Sensor_VType::SENSOR_TYPE_STRING:           return F("String");
    case Sensor_VType::SENSOR_TYPE_NONE:             return F("None");
    case Sensor_VType::SENSOR_TYPE_NOT_SET:  break;
  }
  return F("");
}

bool isSimpleOutputDataType(Sensor_VType sensorType)
{
  return sensorType == Sensor_VType::SENSOR_TYPE_SINGLE ||
         sensorType == Sensor_VType::SENSOR_TYPE_DUAL   ||
         sensorType == Sensor_VType::SENSOR_TYPE_TRIPLE ||
         sensorType == Sensor_VType::SENSOR_TYPE_QUAD;
}

bool isULongOutputDataType(Sensor_VType sensorType)
{
  return sensorType == Sensor_VType::SENSOR_TYPE_ULONG_SINGLE ||
         sensorType == Sensor_VType::SENSOR_TYPE_ULONG_DUAL   ||
         sensorType == Sensor_VType::SENSOR_TYPE_ULONG_TRIPLE ||
         sensorType == Sensor_VType::SENSOR_TYPE_ULONG_QUAD;
}

bool isLongOutputDataType(Sensor_VType sensorType)
{
  return sensorType == Sensor_VType::SENSOR_TYPE_LONG ||
         sensorType == Sensor_VType::SENSOR_TYPE_LONG_DUAL   ||
         sensorType == Sensor_VType::SENSOR_TYPE_LONG_TRIPLE ||
         sensorType == Sensor_VType::SENSOR_TYPE_LONG_QUAD;
}

bool isUInt64OutputDataType(Sensor_VType sensorType)
{
  return sensorType == Sensor_VType::SENSOR_TYPE_UINT64_SINGLE ||
         sensorType == Sensor_VType::SENSOR_TYPE_UINT64_DUAL;
}

bool isInt64OutputDataType(Sensor_VType sensorType)
{
  return sensorType == Sensor_VType::SENSOR_TYPE_INT64_SINGLE ||
         sensorType == Sensor_VType::SENSOR_TYPE_INT64_DUAL;
}

bool isFloatOutputDataType(Sensor_VType sensorType)
{
  return !isIntegerOutputDataType(sensorType) &&
         !isDoubleOutputDataType(sensorType);
}

bool isDoubleOutputDataType(Sensor_VType sensorType)
{
  return sensorType == Sensor_VType::SENSOR_TYPE_DOUBLE_SINGLE ||
         sensorType == Sensor_VType::SENSOR_TYPE_DOUBLE_DUAL;
}

bool isIntegerOutputDataType(Sensor_VType sensorType)
{
  return isULongOutputDataType(sensorType)  ||
         isLongOutputDataType(sensorType)   ||
         isUInt64OutputDataType(sensorType) ||
         isInt64OutputDataType(sensorType);
}

bool is32bitOutputDataType(Sensor_VType sensorType)
{
  if (isUInt64OutputDataType(sensorType) ||
      isInt64OutputDataType(sensorType) ||
      isDoubleOutputDataType(sensorType) ||
      (sensorType == Sensor_VType::SENSOR_TYPE_STRING)) {
    return false;
  }
  return true;
}