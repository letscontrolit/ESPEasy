#include "../DataTypes/TaskValues_Data.h"

#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter_Numerical.h"

TaskValues_Data_t::TaskValues_Data_t() {
  ZERO_FILL(binary);
}

TaskValues_Data_t::TaskValues_Data_t(const TaskValues_Data_t& other)
{
  memcpy(binary, other.binary, sizeof(binary));
}

TaskValues_Data_t& TaskValues_Data_t::operator=(const TaskValues_Data_t& other)
{
  memcpy(binary, other.binary, sizeof(binary));
  return *this;
}

void TaskValues_Data_t::clear() {
  ZERO_FILL(binary);
}

void TaskValues_Data_t::copyValue(const TaskValues_Data_t& other, uint8_t varNr, Sensor_VType sensorType)
{
  if (sensorType != Sensor_VType::SENSOR_TYPE_STRING) {
    if (is32bitOutputDataType(sensorType)) {
      if (varNr < VARS_PER_TASK) {
        uint32s[varNr] = other.uint32s[varNr];
      }
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
    } else {
      if ((varNr < (VARS_PER_TASK / 2))) {
        uint64s[varNr] = other.uint64s[varNr];
      }
#endif
    }
  }
}

unsigned long TaskValues_Data_t::getSensorTypeLong() const
{
  const uint16_t low   = floats[0];
  const uint16_t high  = floats[1];
  unsigned long  value = high;

  value <<= 16;
  value  |= low;
  return value;
}

void TaskValues_Data_t::setSensorTypeLong(unsigned long value)
{
  floats[0] = value & 0xFFFF;
  floats[1] = (value >> 16) & 0xFFFF;
}

#if FEATURE_EXTENDED_TASK_VALUE_TYPES

int32_t TaskValues_Data_t::getInt32(uint8_t varNr) const
{
  if (varNr < VARS_PER_TASK) {
    return int32s[varNr];
  }
  return 0;
}

void TaskValues_Data_t::setInt32(uint8_t varNr, int32_t value)
{
  if (varNr < VARS_PER_TASK) {
    int32s[varNr] = value;
  }
}
#endif

uint32_t TaskValues_Data_t::getUint32(uint8_t varNr) const
{
  if (varNr < VARS_PER_TASK) {
    return uint32s[varNr];
  }
  return 0u;
}

void TaskValues_Data_t::setUint32(uint8_t varNr, uint32_t value)
{
  if (varNr < VARS_PER_TASK) {
    uint32s[varNr] = value;
  }
}

#if FEATURE_EXTENDED_TASK_VALUE_TYPES

int64_t TaskValues_Data_t::getInt64(uint8_t varNr) const
{
  if ((varNr < (VARS_PER_TASK / 2))) {
    return int64s[varNr];
  }
  return 0;
}

void TaskValues_Data_t::setInt64(uint8_t varNr, int64_t value)
{
  if ((varNr < (VARS_PER_TASK / 2))) {
    int64s[varNr] = value;
  }
}

uint64_t TaskValues_Data_t::getUint64(uint8_t varNr) const
{
  if ((varNr < (VARS_PER_TASK / 2))) {
    return uint64s[varNr];
  }
  return 0u;
}

void TaskValues_Data_t::setUint64(uint8_t varNr, uint64_t value)
{
  if ((varNr < (VARS_PER_TASK / 2))) {
    uint64s[varNr] = value;
  }
}
#endif

float TaskValues_Data_t::getFloat(uint8_t varNr) const
{
  if (varNr < VARS_PER_TASK) {
    return floats[varNr];
  }
  return 0.0f;
}

void TaskValues_Data_t::setFloat(uint8_t varNr, float  value)
{
  if (varNr < VARS_PER_TASK) {
    floats[varNr] = value;
  }
}

#if FEATURE_EXTENDED_TASK_VALUE_TYPES
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
double TaskValues_Data_t::getDouble(uint8_t varNr) const
{
  if ((varNr < (VARS_PER_TASK / 2))) {
    return doubles[varNr];
  }
  return 0.0;
}

void TaskValues_Data_t::setDouble(uint8_t varNr, double  value)
{
  if ((varNr < (VARS_PER_TASK / 2))) {
    doubles[varNr] = value;
  }
}
#endif
#endif

ESPEASY_RULES_FLOAT_TYPE TaskValues_Data_t::getAsDouble(uint8_t varNr, Sensor_VType sensorType) const
{
  if (sensorType == Sensor_VType::SENSOR_TYPE_ULONG) {
    return getSensorTypeLong();
  } else if (isFloatOutputDataType(sensorType)) {
    return getFloat(varNr);
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
  } else if (isUInt32OutputDataType(sensorType)) {
    return getUint32(varNr);
  } else if (isInt32OutputDataType(sensorType)) {
    return getInt32(varNr);
  } else if (isUInt64OutputDataType(sensorType)) {
    return getUint64(varNr);
  } else if (isInt64OutputDataType(sensorType)) {
    return getInt64(varNr);
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  } else if (isDoubleOutputDataType(sensorType)) {
    return getDouble(varNr);
#endif    
#endif
  }
  return 0.0;
}

void TaskValues_Data_t::set(uint8_t varNr, const ESPEASY_RULES_FLOAT_TYPE& value, Sensor_VType sensorType)
{
  if (sensorType == Sensor_VType::SENSOR_TYPE_ULONG) {
    // Legacy formatting the old "SENSOR_TYPE_ULONG" type
    setSensorTypeLong(value);
  } else if (isFloatOutputDataType(sensorType)) {
    setFloat(varNr, value);
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
  } else if (isUInt32OutputDataType(sensorType)) {
    setUint32(varNr, value);
  } else if (isInt32OutputDataType(sensorType)) {
    setInt32(varNr, value);
  } else if (isUInt64OutputDataType(sensorType)) {
    setUint64(varNr, value);
  } else if (isInt64OutputDataType(sensorType)) {
    setInt64(varNr, value);
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  } else if (isDoubleOutputDataType(sensorType)) {
    setDouble(varNr, value);
#endif
#endif
  }
}

bool TaskValues_Data_t::isValid(uint8_t varNr, Sensor_VType  sensorType) const
{
  if (sensorType == Sensor_VType::SENSOR_TYPE_NONE) {
    return false;
  } else if (isFloatOutputDataType(sensorType)) {
    return isValidFloat(getFloat(varNr));
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  } else if (isDoubleOutputDataType(sensorType)) {
    return isValidDouble(getDouble(varNr));
#endif
#endif
  }
  return true;
}

String TaskValues_Data_t::getAsString(uint8_t varNr, Sensor_VType  sensorType, uint8_t nrDecimals) const
{
  String result;

  if (isFloatOutputDataType(sensorType)) {
    result = toString(getFloat(varNr), nrDecimals);
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  } else if (isDoubleOutputDataType(sensorType)) {
    result = doubleToString(getDouble(varNr), nrDecimals);
#endif
#endif
  } else if (sensorType == Sensor_VType::SENSOR_TYPE_ULONG) {
    return String(getSensorTypeLong());
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
  } else if (isUInt32OutputDataType(sensorType)) {
    return String(getUint32(varNr));
  } else if (isInt32OutputDataType(sensorType)) {
    return String(getInt32(varNr));
  } else if (isUInt64OutputDataType(sensorType)) {
    return ull2String(getUint64(varNr));
  } else if (isInt64OutputDataType(sensorType)) {
    return ll2String(getInt64(varNr));
#endif
  }
  result.trim();
  return result;
}
