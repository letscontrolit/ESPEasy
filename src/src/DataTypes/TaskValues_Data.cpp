#include "../DataTypes/TaskValues_Data.h"

#include "../Helpers/StringConverter_Numerical.h"

TaskValues_Data_t::TaskValues_Data_t() {
  ZERO_FILL(binary);
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

double TaskValues_Data_t::getAsDouble(uint8_t varNr, Sensor_VType sensorType) const
{
  if (sensorType == Sensor_VType::SENSOR_TYPE_LONG) {
    return getSensorTypeLong();
  } else if (isULongOutputDataType(sensorType)) {
    return getUint32(varNr);
  } else if (isLongOutputDataType(sensorType)) {
    return getInt32(varNr);
  } else if (isUInt64OutputDataType(sensorType)) {
    return getUint64(varNr);
  } else if (isInt64OutputDataType(sensorType)) {
    return getInt64(varNr);
  } else if (isDoubleOutputDataType(sensorType)) {
    return getDouble(varNr);
  }
  return getFloat(varNr);
}

void TaskValues_Data_t::set(uint8_t varNr, const double& value, Sensor_VType sensorType)
{
  if (isULongOutputDataType(sensorType)) {
    setUint32(varNr, value);
  } else if (isLongOutputDataType(sensorType)) {
    if (sensorType == Sensor_VType::SENSOR_TYPE_LONG) {
      // Legacy formatting the old "SENSOR_TYPE_LONG" type
      setSensorTypeLong(value);
    } else {
      setInt32(varNr, value);
    }
  } else if (isUInt64OutputDataType(sensorType)) {
    setUint64(varNr, value);
  } else if (isInt64OutputDataType(sensorType)) {
    setInt64(varNr, value);
  } else if (isDoubleOutputDataType(sensorType)) {
    setDouble(varNr, value);
  } else {
    setFloat(varNr, value);
  }
}

String TaskValues_Data_t::getAsString(uint8_t varNr, Sensor_VType  sensorType, uint8_t nrDecimals) const
{
  String result;
  if (sensorType == Sensor_VType::SENSOR_TYPE_LONG) {
    return String(getSensorTypeLong());
  } else if (isULongOutputDataType(sensorType)) {
    return String(getUint32(varNr));
  } else if (isLongOutputDataType(sensorType)) {
    return String(getInt32(varNr));
  } else if (isUInt64OutputDataType(sensorType)) {
    return ull2String(getUint64(varNr));
  } else if (isInt64OutputDataType(sensorType)) {
    return ll2String(getInt64(varNr));
  } else if (isDoubleOutputDataType(sensorType)) {
    result = doubleToString(getDouble(varNr), nrDecimals);
  } else {
    result = toString(getFloat(varNr), nrDecimals);
  }
  result.trim();
  return result;
}
