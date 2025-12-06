#include "../DataTypes/SensorVType.h"
#include "../Helpers/StringConverter.h"


/*********************************************************************************************\
   Get value count from sensor type

   Only use this function to determine nr of output values when changing output type of a task
   To get the actual output values for a task, use getValueCountForTask
\*********************************************************************************************/
uint8_t getValueCountFromSensorType(Sensor_VType sensorType) {
  return getValueCountFromSensorType(sensorType, true);
}
uint8_t getValueCountFromSensorType(Sensor_VType sensorType, bool log)
{
  switch (sensorType)
  {
    case Sensor_VType::SENSOR_TYPE_NONE:
      return 0;
    case Sensor_VType::SENSOR_TYPE_SINGLE:        // single value sensor, used for Dallas, BH1750, etc
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_INT32_SINGLE:  // 1x int32_t
    case Sensor_VType::SENSOR_TYPE_UINT64_SINGLE: // 1x uint64_t
    case Sensor_VType::SENSOR_TYPE_INT64_SINGLE:  // 1x int64_t
    case Sensor_VType::SENSOR_TYPE_DOUBLE_SINGLE: // 1x ESPEASY_RULES_FLOAT_TYPE
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_SWITCH:
    case Sensor_VType::SENSOR_TYPE_DIMMER:
    case Sensor_VType::SENSOR_TYPE_ULONG:  // single unsigned LONG value, stored in two floats (rfid tags)
    case Sensor_VType::SENSOR_TYPE_STRING: // String type data stored in the event->String2
      return 1;
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM:
    case Sensor_VType::SENSOR_TYPE_TEMP_BARO:
    case Sensor_VType::SENSOR_TYPE_DUAL:        // 2x float
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_UINT32_DUAL: // 2x uint32_t
    case Sensor_VType::SENSOR_TYPE_INT32_DUAL:  // 2x int32_t
    case Sensor_VType::SENSOR_TYPE_UINT64_DUAL: // 2x uint64_t
    case Sensor_VType::SENSOR_TYPE_INT64_DUAL:  // 2x int64_t
    case Sensor_VType::SENSOR_TYPE_DOUBLE_DUAL: // 2x ESPEASY_RULES_FLOAT_TYPE
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
      return 2;
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO:
    case Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO: // Values 1 and 3 will contain data.
    case Sensor_VType::SENSOR_TYPE_TRIPLE:          // 3x float
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_UINT32_TRIPLE:   // 3x uint32_t
    case Sensor_VType::SENSOR_TYPE_INT32_TRIPLE:    // 3x int32_t
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_WIND:
      return 3;
    case Sensor_VType::SENSOR_TYPE_QUAD:        // 4x float
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_UINT32_QUAD: // 4x uint32_t
    case Sensor_VType::SENSOR_TYPE_INT32_QUAD:  // 4x int32_t
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
      return 4;
    case Sensor_VType::SENSOR_TYPE_NOT_SET:  break;

    // FIXME To be ignored?
    case Sensor_VType::SENSOR_TYPE_ANALOG_ONLY:
    case Sensor_VType::SENSOR_TYPE_TEMP_ONLY:
    case Sensor_VType::SENSOR_TYPE_HUM_ONLY:
    case Sensor_VType::SENSOR_TYPE_LUX_ONLY:
    case Sensor_VType::SENSOR_TYPE_DISTANCE_ONLY:
    case Sensor_VType::SENSOR_TYPE_DIRECTION_ONLY:
    case Sensor_VType::SENSOR_TYPE_DUSTPM2_5_ONLY:
    case Sensor_VType::SENSOR_TYPE_DUSTPM1_0_ONLY:
    case Sensor_VType::SENSOR_TYPE_DUSTPM10_ONLY:
    case Sensor_VType::SENSOR_TYPE_MOISTURE_ONLY:
    case Sensor_VType::SENSOR_TYPE_CO2_ONLY:
    case Sensor_VType::SENSOR_TYPE_GPS_ONLY:
    case Sensor_VType::SENSOR_TYPE_UV_ONLY:
    case Sensor_VType::SENSOR_TYPE_UV_INDEX_ONLY:
    case Sensor_VType::SENSOR_TYPE_IR_ONLY:
    case Sensor_VType::SENSOR_TYPE_WEIGHT_ONLY:
    case Sensor_VType::SENSOR_TYPE_VOLTAGE_ONLY:
    case Sensor_VType::SENSOR_TYPE_CURRENT_ONLY:
    case Sensor_VType::SENSOR_TYPE_POWER_USG_ONLY:
    case Sensor_VType::SENSOR_TYPE_POWER_FACT_ONLY:
    case Sensor_VType::SENSOR_TYPE_APPRNT_POWER_USG_ONLY:
    case Sensor_VType::SENSOR_TYPE_TVOC_ONLY:
    case Sensor_VType::SENSOR_TYPE_BARO_ONLY:
    case Sensor_VType::SENSOR_TYPE_COLOR_RED_ONLY:
    case Sensor_VType::SENSOR_TYPE_COLOR_GREEN_ONLY:
    case Sensor_VType::SENSOR_TYPE_COLOR_BLUE_ONLY:
    case Sensor_VType::SENSOR_TYPE_COLOR_TEMP_ONLY:
    case Sensor_VType::SENSOR_TYPE_REACTIVE_POWER_ONLY:
    case Sensor_VType::SENSOR_TYPE_AQI_ONLY:
    case Sensor_VType::SENSOR_TYPE_NOX_ONLY:
    case Sensor_VType::SENSOR_TYPE_SWITCH_INVERTED:
    case Sensor_VType::SENSOR_TYPE_WIND_SPEED:
    case Sensor_VType::SENSOR_TYPE_DURATION:
    case Sensor_VType::SENSOR_TYPE_DATE:
    case Sensor_VType::SENSOR_TYPE_TIMESTAMP:
    case Sensor_VType::SENSOR_TYPE_DATA_RATE:
    case Sensor_VType::SENSOR_TYPE_DATA_SIZE:
    case Sensor_VType::SENSOR_TYPE_SOUND_PRESSURE:
    case Sensor_VType::SENSOR_TYPE_SIGNAL_STRENGTH:
    case Sensor_VType::SENSOR_TYPE_REACTIVE_ENERGY:
    case Sensor_VType::SENSOR_TYPE_FREQUENCY:
    case Sensor_VType::SENSOR_TYPE_ENERGY:
    case Sensor_VType::SENSOR_TYPE_ENERGY_STORAGE:
    case Sensor_VType::SENSOR_TYPE_ABS_HUMIDITY:
    case Sensor_VType::SENSOR_TYPE_ATMOS_PRESSURE:
    case Sensor_VType::SENSOR_TYPE_BLOOD_GLUCOSE_C:
    case Sensor_VType::SENSOR_TYPE_CO_ONLY:
    case Sensor_VType::SENSOR_TYPE_ENERGY_DISTANCE:
    case Sensor_VType::SENSOR_TYPE_GAS_ONLY:
    case Sensor_VType::SENSOR_TYPE_NITROUS_OXIDE:
    case Sensor_VType::SENSOR_TYPE_OZONE_ONLY:
    case Sensor_VType::SENSOR_TYPE_PRECIPITATION:
    case Sensor_VType::SENSOR_TYPE_PRECIPITATION_INTEN:
    case Sensor_VType::SENSOR_TYPE_SULPHUR_DIOXIDE:
    case Sensor_VType::SENSOR_TYPE_VOC_PARTS:
    case Sensor_VType::SENSOR_TYPE_VOLUME:
    case Sensor_VType::SENSOR_TYPE_VOLUME_FLOW_RATE:
    case Sensor_VType::SENSOR_TYPE_VOLUME_STORAGE:
    case Sensor_VType::SENSOR_TYPE_WATER:
      return 1;
  }
  #ifndef BUILD_NO_DEBUG
  if (log) {
    addLog(LOG_LEVEL_ERROR, F("getValueCountFromSensorType: Unknown sensortype"));
  }
  #endif // ifndef BUILD_NO_DEBUG
  return 0;
}

const __FlashStringHelper* getSensorTypeLabel(Sensor_VType sensorType) {
  switch (sensorType) {
    case Sensor_VType::SENSOR_TYPE_SWITCH:           return F("Switch");
    case Sensor_VType::SENSOR_TYPE_DIMMER:           return F("Dimmer");
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM:         return F("Temp / Hum");
    case Sensor_VType::SENSOR_TYPE_TEMP_BARO:        return F("Temp / Baro");
    case Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO:  return F("Temp / - / Baro");
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO:    return F("Temp / Hum / Baro");
    case Sensor_VType::SENSOR_TYPE_SINGLE:           return F("Single");
    case Sensor_VType::SENSOR_TYPE_DUAL:             return F("Dual");
    case Sensor_VType::SENSOR_TYPE_TRIPLE:           return F("Triple");
    case Sensor_VType::SENSOR_TYPE_QUAD:             return F("Quad");
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_INT32_SINGLE:     return F("Int32 (1x)");
    case Sensor_VType::SENSOR_TYPE_INT32_DUAL:       return F("Int32 (2x)");
    case Sensor_VType::SENSOR_TYPE_INT32_TRIPLE:     return F("Int32 (3x)");
    case Sensor_VType::SENSOR_TYPE_INT32_QUAD:       return F("Int32 (4x)");
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_ULONG:            return F("UInt32 (1x)");
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_UINT32_DUAL:      return F("UInt32 (2x)");
    case Sensor_VType::SENSOR_TYPE_UINT32_TRIPLE:    return F("UInt32 (3x)");
    case Sensor_VType::SENSOR_TYPE_UINT32_QUAD:      return F("UInt32 (4x)");
    case Sensor_VType::SENSOR_TYPE_UINT64_SINGLE:    return F("UInt64 (1x)");
    case Sensor_VType::SENSOR_TYPE_UINT64_DUAL:      return F("UInt64 (2x)");
    case Sensor_VType::SENSOR_TYPE_INT64_SINGLE:     return F("Int64 (1x)");
    case Sensor_VType::SENSOR_TYPE_INT64_DUAL:       return F("Int64 (2x)");
    case Sensor_VType::SENSOR_TYPE_DOUBLE_SINGLE:    return F("Double (1x)");
    case Sensor_VType::SENSOR_TYPE_DOUBLE_DUAL:      return F("Double (2x)");
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_WIND:             return F("Wind");
    case Sensor_VType::SENSOR_TYPE_STRING:           return F("String");
    case Sensor_VType::SENSOR_TYPE_NONE:             return F("None");
    case Sensor_VType::SENSOR_TYPE_NOT_SET:  break;

      // FIXME To be ignored?
    #if FEATURE_MQTT_DISCOVER || FEATURE_CUSTOM_TASKVAR_VTYPE
    case Sensor_VType::SENSOR_TYPE_ANALOG_ONLY:      return F("Analog");
    case Sensor_VType::SENSOR_TYPE_TEMP_ONLY:        return F("Temp");
    case Sensor_VType::SENSOR_TYPE_HUM_ONLY:         return F("Hum");
    case Sensor_VType::SENSOR_TYPE_LUX_ONLY:         return F("Lux");
    case Sensor_VType::SENSOR_TYPE_DISTANCE_ONLY:    return F("Distance");
    case Sensor_VType::SENSOR_TYPE_DIRECTION_ONLY:   return F("Direction");
    case Sensor_VType::SENSOR_TYPE_DUSTPM2_5_ONLY:   return F("Dust PM2.5");
    case Sensor_VType::SENSOR_TYPE_DUSTPM1_0_ONLY:   return F("Dust PM1.0");
    case Sensor_VType::SENSOR_TYPE_DUSTPM10_ONLY:    return F("Dust PM10");
    case Sensor_VType::SENSOR_TYPE_MOISTURE_ONLY:    return F("Moisture");
    case Sensor_VType::SENSOR_TYPE_CO2_ONLY:         return F("(e)CO2");
    case Sensor_VType::SENSOR_TYPE_GPS_ONLY:         return F("GPS");
    case Sensor_VType::SENSOR_TYPE_UV_ONLY:          return F("UV");
    case Sensor_VType::SENSOR_TYPE_UV_INDEX_ONLY:    return F("UV Index");
    case Sensor_VType::SENSOR_TYPE_IR_ONLY:          return F("IR");
    case Sensor_VType::SENSOR_TYPE_WEIGHT_ONLY:      return F("Weight");
    case Sensor_VType::SENSOR_TYPE_VOLTAGE_ONLY:     return F("Voltage");
    case Sensor_VType::SENSOR_TYPE_CURRENT_ONLY:     return F("Current");
    case Sensor_VType::SENSOR_TYPE_POWER_USG_ONLY:   return F("Power Usage");
    case Sensor_VType::SENSOR_TYPE_POWER_FACT_ONLY:  return F("Power Factor");
    case Sensor_VType::SENSOR_TYPE_APPRNT_POWER_USG_ONLY: return F("Apparent Power Usage");
    case Sensor_VType::SENSOR_TYPE_TVOC_ONLY:        return F("TVOC");
    case Sensor_VType::SENSOR_TYPE_BARO_ONLY:        return F("Baro");
    case Sensor_VType::SENSOR_TYPE_COLOR_RED_ONLY:   return F("Red");
    case Sensor_VType::SENSOR_TYPE_COLOR_GREEN_ONLY: return F("Green");
    case Sensor_VType::SENSOR_TYPE_COLOR_BLUE_ONLY:  return F("Blue");
    case Sensor_VType::SENSOR_TYPE_COLOR_TEMP_ONLY:  return F("Color temperature");
    case Sensor_VType::SENSOR_TYPE_REACTIVE_POWER_ONLY: return F("Reactive Power");
    case Sensor_VType::SENSOR_TYPE_AQI_ONLY:         return F("AQI");
    case Sensor_VType::SENSOR_TYPE_NOX_ONLY:         return F("NOx");
    case Sensor_VType::SENSOR_TYPE_SWITCH_INVERTED:  return F("Switch (inv.)");
    case Sensor_VType::SENSOR_TYPE_WIND_SPEED:       return F("Wind speed");
    case Sensor_VType::SENSOR_TYPE_DURATION:         return F("Duration");
    case Sensor_VType::SENSOR_TYPE_DATE:             return F("Date");
    case Sensor_VType::SENSOR_TYPE_TIMESTAMP:        return F("Timestamp");
    case Sensor_VType::SENSOR_TYPE_DATA_RATE:        return F("Data rate");
    case Sensor_VType::SENSOR_TYPE_DATA_SIZE:        return F("Data size");
    case Sensor_VType::SENSOR_TYPE_SOUND_PRESSURE:   return F("Sound pressure");
    case Sensor_VType::SENSOR_TYPE_SIGNAL_STRENGTH:  return F("Signal strength");
    case Sensor_VType::SENSOR_TYPE_REACTIVE_ENERGY:  return F("Reactive Energy");
    case Sensor_VType::SENSOR_TYPE_FREQUENCY:        return F("Frequency");
    case Sensor_VType::SENSOR_TYPE_ENERGY:           return F("Energy");
    case Sensor_VType::SENSOR_TYPE_ENERGY_STORAGE:   return F("Energy storage");
    case Sensor_VType::SENSOR_TYPE_ABS_HUMIDITY:     return F("Absolute humidity");
    case Sensor_VType::SENSOR_TYPE_ATMOS_PRESSURE:   return F("Atmospheric pressure");
    case Sensor_VType::SENSOR_TYPE_BLOOD_GLUCOSE_C:  return F("Blood glucose conc.");
    case Sensor_VType::SENSOR_TYPE_CO_ONLY:          return F("CO");
    case Sensor_VType::SENSOR_TYPE_ENERGY_DISTANCE:  return F("Energy distance");
    case Sensor_VType::SENSOR_TYPE_GAS_ONLY:         return F("Gas");
    case Sensor_VType::SENSOR_TYPE_NITROUS_OXIDE:    return F("N2O");
    case Sensor_VType::SENSOR_TYPE_OZONE_ONLY:       return F("Ozone");
    case Sensor_VType::SENSOR_TYPE_PRECIPITATION:    return F("Precipitation");
    case Sensor_VType::SENSOR_TYPE_PRECIPITATION_INTEN: return F("Precipitation intensity");
    case Sensor_VType::SENSOR_TYPE_SULPHUR_DIOXIDE:  return F("SO2");
    case Sensor_VType::SENSOR_TYPE_VOC_PARTS:        return F("VOC parts");
    case Sensor_VType::SENSOR_TYPE_VOLUME:           return F("Volume");
    case Sensor_VType::SENSOR_TYPE_VOLUME_FLOW_RATE: return F("Volume flow rate");
    case Sensor_VType::SENSOR_TYPE_VOLUME_STORAGE:   return F("Volume storage");
    case Sensor_VType::SENSOR_TYPE_WATER:            return F("Water cons.");
    #else // if FEATURE_MQTT_DISCOVER || FEATURE_CUSTOM_TASKVAR_VTYPE
    case Sensor_VType::SENSOR_TYPE_ANALOG_ONLY:
    case Sensor_VType::SENSOR_TYPE_TEMP_ONLY:
    case Sensor_VType::SENSOR_TYPE_HUM_ONLY:
    case Sensor_VType::SENSOR_TYPE_LUX_ONLY:
    case Sensor_VType::SENSOR_TYPE_DISTANCE_ONLY:
    case Sensor_VType::SENSOR_TYPE_DIRECTION_ONLY:
    case Sensor_VType::SENSOR_TYPE_DUSTPM2_5_ONLY:
    case Sensor_VType::SENSOR_TYPE_DUSTPM1_0_ONLY:
    case Sensor_VType::SENSOR_TYPE_DUSTPM10_ONLY:
    case Sensor_VType::SENSOR_TYPE_MOISTURE_ONLY:
    case Sensor_VType::SENSOR_TYPE_CO2_ONLY:
    case Sensor_VType::SENSOR_TYPE_GPS_ONLY:
    case Sensor_VType::SENSOR_TYPE_UV_ONLY:
    case Sensor_VType::SENSOR_TYPE_UV_INDEX_ONLY:
    case Sensor_VType::SENSOR_TYPE_IR_ONLY:
    case Sensor_VType::SENSOR_TYPE_WEIGHT_ONLY:
    case Sensor_VType::SENSOR_TYPE_VOLTAGE_ONLY:
    case Sensor_VType::SENSOR_TYPE_CURRENT_ONLY:
    case Sensor_VType::SENSOR_TYPE_POWER_USG_ONLY:
    case Sensor_VType::SENSOR_TYPE_POWER_FACT_ONLY:
    case Sensor_VType::SENSOR_TYPE_APPRNT_POWER_USG_ONLY:
    case Sensor_VType::SENSOR_TYPE_TVOC_ONLY:
    case Sensor_VType::SENSOR_TYPE_BARO_ONLY:
    case Sensor_VType::SENSOR_TYPE_COLOR_RED_ONLY:
    case Sensor_VType::SENSOR_TYPE_COLOR_GREEN_ONLY:
    case Sensor_VType::SENSOR_TYPE_COLOR_BLUE_ONLY:
    case Sensor_VType::SENSOR_TYPE_COLOR_TEMP_ONLY:
    case Sensor_VType::SENSOR_TYPE_REACTIVE_POWER_ONLY:
    case Sensor_VType::SENSOR_TYPE_AQI_ONLY:
    case Sensor_VType::SENSOR_TYPE_NOX_ONLY:
    case Sensor_VType::SENSOR_TYPE_SWITCH_INVERTED:
    case Sensor_VType::SENSOR_TYPE_WIND_SPEED:
    case Sensor_VType::SENSOR_TYPE_DURATION:
    case Sensor_VType::SENSOR_TYPE_DATE:
    case Sensor_VType::SENSOR_TYPE_TIMESTAMP:
    case Sensor_VType::SENSOR_TYPE_DATA_RATE:
    case Sensor_VType::SENSOR_TYPE_DATA_SIZE:
    case Sensor_VType::SENSOR_TYPE_SOUND_PRESSURE:
    case Sensor_VType::SENSOR_TYPE_SIGNAL_STRENGTH:
    case Sensor_VType::SENSOR_TYPE_REACTIVE_ENERGY:
    case Sensor_VType::SENSOR_TYPE_FREQUENCY:
    case Sensor_VType::SENSOR_TYPE_ENERGY:
    case Sensor_VType::SENSOR_TYPE_ENERGY_STORAGE:
    case Sensor_VType::SENSOR_TYPE_ABS_HUMIDITY:
    case Sensor_VType::SENSOR_TYPE_ATMOS_PRESSURE:
    case Sensor_VType::SENSOR_TYPE_BLOOD_GLUCOSE_C:
    case Sensor_VType::SENSOR_TYPE_CO_ONLY:
    case Sensor_VType::SENSOR_TYPE_ENERGY_DISTANCE:
    case Sensor_VType::SENSOR_TYPE_GAS_ONLY:
    case Sensor_VType::SENSOR_TYPE_NITROUS_OXIDE:
    case Sensor_VType::SENSOR_TYPE_OZONE_ONLY:
    case Sensor_VType::SENSOR_TYPE_PRECIPITATION:
    case Sensor_VType::SENSOR_TYPE_PRECIPITATION_INTEN:
    case Sensor_VType::SENSOR_TYPE_SULPHUR_DIOXIDE:
    case Sensor_VType::SENSOR_TYPE_VOC_PARTS:
    case Sensor_VType::SENSOR_TYPE_VOLUME:
    case Sensor_VType::SENSOR_TYPE_VOLUME_FLOW_RATE:
    case Sensor_VType::SENSOR_TYPE_VOLUME_STORAGE:
    case Sensor_VType::SENSOR_TYPE_WATER:
      break;
    #endif // if FEATURE_MQTT_DISCOVER || FEATURE_CUSTOM_TASKVAR_VTYPE
  }
  return F("");
}

#if FEATURE_CUSTOM_TASKVAR_VTYPE
bool isMQTTDiscoverySensorType(Sensor_VType sensorType)
{
  switch (sensorType)
  {
    case Sensor_VType::SENSOR_TYPE_NONE:
    case Sensor_VType::SENSOR_TYPE_SINGLE:        // single value sensor, used for Dallas, BH1750, etc
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_INT32_SINGLE:  // 1x int32_t
    case Sensor_VType::SENSOR_TYPE_UINT64_SINGLE: // 1x uint64_t
    case Sensor_VType::SENSOR_TYPE_INT64_SINGLE:  // 1x int64_t
    case Sensor_VType::SENSOR_TYPE_DOUBLE_SINGLE: // 1x ESPEASY_RULES_FLOAT_TYPE
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_ULONG:  // single unsigned LONG value, stored in two floats (rfid tags)
    case Sensor_VType::SENSOR_TYPE_STRING: // String type data stored in the event->String2
    case Sensor_VType::SENSOR_TYPE_DUAL:        // 2x float
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_UINT32_DUAL: // 2x uint32_t
    case Sensor_VType::SENSOR_TYPE_INT32_DUAL:  // 2x int32_t
    case Sensor_VType::SENSOR_TYPE_UINT64_DUAL: // 2x uint64_t
    case Sensor_VType::SENSOR_TYPE_INT64_DUAL:  // 2x int64_t
    case Sensor_VType::SENSOR_TYPE_DOUBLE_DUAL: // 2x ESPEASY_RULES_FLOAT_TYPE
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_TRIPLE:          // 3x float
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_UINT32_TRIPLE:   // 3x uint32_t
    case Sensor_VType::SENSOR_TYPE_INT32_TRIPLE:    // 3x int32_t
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_QUAD:        // 4x float
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_UINT32_QUAD: // 4x uint32_t
    case Sensor_VType::SENSOR_TYPE_INT32_QUAD:  // 4x int32_t
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
    case Sensor_VType::SENSOR_TYPE_NOT_SET:
    case Sensor_VType::SENSOR_TYPE_ANALOG_ONLY: // TODO Implement discovery
    case Sensor_VType::SENSOR_TYPE_DIMMER:      // TODO Implement discovery
    case Sensor_VType::SENSOR_TYPE_GPS_ONLY:    // TODO Implement discovery
      break;

    // All value types that are used in MQTT AutoDiscovery
    case Sensor_VType::SENSOR_TYPE_SWITCH:
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM:
    case Sensor_VType::SENSOR_TYPE_TEMP_BARO:
    case Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO:
    case Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO: // Values 1 and 3 will contain data.
    case Sensor_VType::SENSOR_TYPE_WIND:
    case Sensor_VType::SENSOR_TYPE_TEMP_ONLY:
    case Sensor_VType::SENSOR_TYPE_HUM_ONLY:
    case Sensor_VType::SENSOR_TYPE_LUX_ONLY:
    case Sensor_VType::SENSOR_TYPE_DISTANCE_ONLY:
    case Sensor_VType::SENSOR_TYPE_DIRECTION_ONLY:
    case Sensor_VType::SENSOR_TYPE_DUSTPM2_5_ONLY:
    case Sensor_VType::SENSOR_TYPE_DUSTPM1_0_ONLY:
    case Sensor_VType::SENSOR_TYPE_DUSTPM10_ONLY:
    case Sensor_VType::SENSOR_TYPE_MOISTURE_ONLY:
    case Sensor_VType::SENSOR_TYPE_CO2_ONLY:
    case Sensor_VType::SENSOR_TYPE_UV_ONLY:
    case Sensor_VType::SENSOR_TYPE_UV_INDEX_ONLY:
    case Sensor_VType::SENSOR_TYPE_IR_ONLY:
    case Sensor_VType::SENSOR_TYPE_WEIGHT_ONLY:
    case Sensor_VType::SENSOR_TYPE_VOLTAGE_ONLY:
    case Sensor_VType::SENSOR_TYPE_CURRENT_ONLY:
    case Sensor_VType::SENSOR_TYPE_POWER_USG_ONLY:
    case Sensor_VType::SENSOR_TYPE_POWER_FACT_ONLY:
    case Sensor_VType::SENSOR_TYPE_APPRNT_POWER_USG_ONLY:
    case Sensor_VType::SENSOR_TYPE_TVOC_ONLY:
    case Sensor_VType::SENSOR_TYPE_BARO_ONLY:
    case Sensor_VType::SENSOR_TYPE_COLOR_RED_ONLY:
    case Sensor_VType::SENSOR_TYPE_COLOR_GREEN_ONLY:
    case Sensor_VType::SENSOR_TYPE_COLOR_BLUE_ONLY:
    case Sensor_VType::SENSOR_TYPE_COLOR_TEMP_ONLY:
    case Sensor_VType::SENSOR_TYPE_REACTIVE_POWER_ONLY:
    case Sensor_VType::SENSOR_TYPE_AQI_ONLY:
    case Sensor_VType::SENSOR_TYPE_NOX_ONLY:
    case Sensor_VType::SENSOR_TYPE_SWITCH_INVERTED:
    case Sensor_VType::SENSOR_TYPE_WIND_SPEED:
    case Sensor_VType::SENSOR_TYPE_DURATION:
    case Sensor_VType::SENSOR_TYPE_DATE:
    case Sensor_VType::SENSOR_TYPE_TIMESTAMP:
    case Sensor_VType::SENSOR_TYPE_DATA_RATE:
    case Sensor_VType::SENSOR_TYPE_DATA_SIZE:
    case Sensor_VType::SENSOR_TYPE_SOUND_PRESSURE:
    case Sensor_VType::SENSOR_TYPE_SIGNAL_STRENGTH:
    case Sensor_VType::SENSOR_TYPE_REACTIVE_ENERGY:
    case Sensor_VType::SENSOR_TYPE_FREQUENCY:
    case Sensor_VType::SENSOR_TYPE_ENERGY:
    case Sensor_VType::SENSOR_TYPE_ENERGY_STORAGE:
    case Sensor_VType::SENSOR_TYPE_ABS_HUMIDITY:
    case Sensor_VType::SENSOR_TYPE_ATMOS_PRESSURE:
    case Sensor_VType::SENSOR_TYPE_BLOOD_GLUCOSE_C:
    case Sensor_VType::SENSOR_TYPE_CO_ONLY:
    case Sensor_VType::SENSOR_TYPE_ENERGY_DISTANCE:
    case Sensor_VType::SENSOR_TYPE_GAS_ONLY:
    case Sensor_VType::SENSOR_TYPE_NITROUS_OXIDE:
    case Sensor_VType::SENSOR_TYPE_OZONE_ONLY:
    case Sensor_VType::SENSOR_TYPE_PRECIPITATION:
    case Sensor_VType::SENSOR_TYPE_PRECIPITATION_INTEN:
    case Sensor_VType::SENSOR_TYPE_SULPHUR_DIOXIDE:
    case Sensor_VType::SENSOR_TYPE_VOC_PARTS:
    case Sensor_VType::SENSOR_TYPE_VOLUME:
    case Sensor_VType::SENSOR_TYPE_VOLUME_FLOW_RATE:
    case Sensor_VType::SENSOR_TYPE_VOLUME_STORAGE:
    case Sensor_VType::SENSOR_TYPE_WATER:
      return true;
  }
  return false;
}
#endif // FEATURE_CUSTOM_TASKVAR_VTYPE

bool isSimpleOutputDataType(Sensor_VType sensorType)
{
  return sensorType == Sensor_VType::SENSOR_TYPE_SINGLE ||
         sensorType == Sensor_VType::SENSOR_TYPE_DUAL   ||
         sensorType == Sensor_VType::SENSOR_TYPE_TRIPLE ||
         sensorType == Sensor_VType::SENSOR_TYPE_QUAD;
}

bool isUInt32OutputDataType(Sensor_VType sensorType)
{
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
  return sensorType == Sensor_VType::SENSOR_TYPE_ULONG         ||
         sensorType == Sensor_VType::SENSOR_TYPE_UINT32_DUAL   ||
         sensorType == Sensor_VType::SENSOR_TYPE_UINT32_TRIPLE ||
         sensorType == Sensor_VType::SENSOR_TYPE_UINT32_QUAD;
#else // if FEATURE_EXTENDED_TASK_VALUE_TYPES
  return sensorType == Sensor_VType::SENSOR_TYPE_ULONG;
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
}

#if FEATURE_EXTENDED_TASK_VALUE_TYPES
bool isInt32OutputDataType(Sensor_VType sensorType)
{
  return sensorType == Sensor_VType::SENSOR_TYPE_INT32_SINGLE ||
         sensorType == Sensor_VType::SENSOR_TYPE_INT32_DUAL   ||
         sensorType == Sensor_VType::SENSOR_TYPE_INT32_TRIPLE ||
         sensorType == Sensor_VType::SENSOR_TYPE_INT32_QUAD;
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

#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

bool isFloatOutputDataType(Sensor_VType sensorType)
{
  return sensorType != Sensor_VType::SENSOR_TYPE_NONE &&
         sensorType != Sensor_VType::SENSOR_TYPE_ULONG &&
         sensorType < Sensor_VType::SENSOR_TYPE_STRING;
}

#if FEATURE_EXTENDED_TASK_VALUE_TYPES
bool isDoubleOutputDataType(Sensor_VType sensorType)
{
  return sensorType == Sensor_VType::SENSOR_TYPE_DOUBLE_SINGLE ||
         sensorType == Sensor_VType::SENSOR_TYPE_DOUBLE_DUAL;
}

#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

bool isIntegerOutputDataType(Sensor_VType sensorType)
{
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
  return isUInt32OutputDataType(sensorType)  ||
         isInt32OutputDataType(sensorType)   ||
         isUInt64OutputDataType(sensorType) ||
         isInt64OutputDataType(sensorType);
#else // if FEATURE_EXTENDED_TASK_VALUE_TYPES
  return isUInt32OutputDataType(sensorType);
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
}

bool is32bitOutputDataType(Sensor_VType sensorType)
{
#if FEATURE_EXTENDED_TASK_VALUE_TYPES

  if (isUInt64OutputDataType(sensorType) ||
      isInt64OutputDataType(sensorType) ||
      isDoubleOutputDataType(sensorType) ||
      (sensorType == Sensor_VType::SENSOR_TYPE_STRING)) {
    return false;
  }
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
  return true;
}

# if FEATURE_MQTT && FEATURE_MQTT_DISCOVER
const char mqtt_valueType_ha_deviceclass_names[] PROGMEM = // !! Offset, starting from Sensor_VType::SENSOR_TYPE_ANALOG_ONLY !!
  "|temperature|humidity|illuminance|distance|wind_direction|" // ANALOG_ONLY .. DIRECTION_ONLY
  "pm25|pm1|pm10|mdi:cup-water|carbon_dioxide||" // DUSTPM2_5_ONLY .. GPS_ONLY
  "irradiance|irradiance|irradiance|mdi:scale|" // UV_ONLY .. WEIGHT_ONLY
  "voltage|current|power|power_factor|power|" // VOLTAGE_ONLY .. APPRNT_POWER_USG_ONLY
  "volatile_organic_compounds|pressure|mdi:palette|mdi:palette|mdi:palette|" // TVOC_ONLY .. COLOR_BLUE_ONLY
  "mdi:temperature-kelvin|reactive_power|aqi|nitrogen_dioxide|" // COLOR_TEMP_ONLY .. NOX_ONLY
  "|wind_speed|duration|date|" // SWITCH_INVERTED .. DATE
  "timestamp|data_rate|data_size|sound_pressure|signal_strength|reactive_energy|" // TIMESTAMP .. REACTIVE_ENERGY
  "frequency|energy|energy_storage|" // FREQUENCY .. ENERGY_STORAGE
  "absolute_humidity|atmospheric_pressure|blood_glucose_concentration|" // ABS_HUMIDITY .. ATMOSPH_PRESSURE
  "carbon_monoxide|energy_distance|gas|nitrous_oxide|ozone|" // CO_ONLY .. OZONE_ONLY
  "precipitation|precipitation_intensity|sulphur_dioxide|volatile_organic_compounds_parts|" // PRECIPITATION .. VOC_PARTS
  "volume|volume_flow_rate|volume_storage|water|" // VOLUME .. WATER
  ;

/**
 * getValueType2HADeviceClass: Convert the ValueType to a HA Device Class
 */
String getValueType2HADeviceClass(Sensor_VType sensorType) {
  char tmp[33]{};                                                   // length: volatile_organic_compounds_parts + \0
  int devClassIndex = static_cast<int>(sensorType);
  if (sensorType >= Sensor_VType::SENSOR_TYPE_ANALOG_ONLY) {
    devClassIndex -= static_cast<int>(Sensor_VType::SENSOR_TYPE_ANALOG_ONLY); // Subtract offset
  // } else if (sensorType == Sensor_VType::SENSOR_TYPE_SWITCH) {
  //   return EMPTY_STRING;
  } else if (sensorType == Sensor_VType::SENSOR_TYPE_WIND) {
    return F("wind_speed");
  // } else if (sensorType == Sensor_VType::SENSOR_TYPE_DIMMER) {
  //   return EMPTY_STRING;
  } else {
    return EMPTY_STRING;
  }

  String result(GetTextIndexed(tmp, sizeof(tmp), devClassIndex, mqtt_valueType_ha_deviceclass_names));

  return result;
}

const char mqtt_valueType_default_ha_uom_names[] PROGMEM = // !! Offset, starting from Sensor_VType::SENSOR_TYPE_ANALOG_ONLY !!
  "|°C|%|lx|cm|°|" // ANALOG_ONLY .. DIRECTION_ONLY
  "µg/m³|µg/m³|µg/m³|%|ppm||" // DUSTPM2_5_ONLY .. GPS_ONLY
  "W/m²|UV Index|W/m²|kg|" // UV_ONLY .. WEIGHT_ONLY
  "V|A|W|%|VA|" // VOLTAGE_ONLY .. APPRNT_POWER_USG_ONLY
  "ppd|hPa|lx|lx|lx|" // TVOC_ONLY .. COLOR_BLUE_ONLY
  "K|var||µg/m³|" // COLOR_TEMP_ONLY .. NOX_ONLY
  "|m/s|min||" // SWITCH_INVERTED .. DATE
  "|bit/s|B|dB|dBm|kvar|" // TIMESTAMP .. REACTIVE_ENERGY
  "Hz|kWh|kWh|" // FREQUENCY .. ENERGY_STORAGE
  "g/m³|mbar|mmol/L|" // ABS_HUMIDITY .. BLOOD_GLUCOSE_C
  "ppm|kWh/100km|L|µg/m³|µg/m³|" // CO_ONLY .. OZONE_ONLY
  "mm|mm/h|µg/m³|ppm|" // PRICIPIATION .. VOC_PARTS
  "L|m³/h|L|L|" // VOLUME .. WATER
  ;

/**
 * getValueType2DefaultHAUoM: Convert the Value Type to a default UoM for HA AutoDiscovery
 */
String getValueType2DefaultHAUoM(Sensor_VType sensorType) {
  char tmp[9]{};                                                   // length: UV Index + \0
  int devClassIndex = static_cast<int>(sensorType);
  if (sensorType >= Sensor_VType::SENSOR_TYPE_ANALOG_ONLY) {
    devClassIndex -= static_cast<int>(Sensor_VType::SENSOR_TYPE_ANALOG_ONLY); // Subtract offset
  // } else if (sensorType == Sensor_VType::SENSOR_TYPE_SWITCH) {
  //   return EMPTY_STRING;
  } else if (sensorType == Sensor_VType::SENSOR_TYPE_WIND) {
    return F("m/s");
  // } else if (sensorType == Sensor_VType::SENSOR_TYPE_DIMMER) {
  //   return EMPTY_STRING;
  } else {
    return EMPTY_STRING;
  }

  String result(GetTextIndexed(tmp, sizeof(tmp), devClassIndex, mqtt_valueType_default_ha_uom_names));

  return result;
}
# endif // if FEATURE_MQTT && FEATURE_MQTT_DISCOVER
