#include "../DataStructs/UserVarStruct.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Plugins.h"
#include "../Helpers/_Plugin_SensorTypeHelper.h"
#include "../Helpers/CRC_functions.h"

UserVarStruct::UserVarStruct()
{
  _data.resize(TASKS_MAX);
}

void UserVarStruct::clear()
{
  for (size_t i = 0; i < _data.size(); ++i) {
    _data[i].clear();
  }
}

// Implementation of [] operator.  This function must return a
// reference as array element can be put on left side
float& UserVarStruct::operator[](unsigned int index)
{
  const unsigned int taskIndex = index / VARS_PER_TASK;
  const unsigned int varNr     = index % VARS_PER_TASK;

  if (taskIndex >= _data.size()) {
    static float errorvalue = NAN;
    addLog(LOG_LEVEL_ERROR, F("UserVar index out of range"));
    return errorvalue;
  }
  return _data.at(taskIndex).floats[varNr];
}

const float& UserVarStruct::operator[](unsigned int index) const
{
  const unsigned int taskIndex = index / VARS_PER_TASK;
  const unsigned int varNr     = index % VARS_PER_TASK;

  if (taskIndex >= _data.size()) {
    static float errorvalue = NAN;
    addLog(LOG_LEVEL_ERROR, F("UserVar index out of range"));
    return errorvalue;
  }
  return _data.at(taskIndex).floats[varNr];
}

unsigned long UserVarStruct::getSensorTypeLong(taskIndex_t taskIndex) const
{
  if (taskIndex < _data.size()) {
    return _data[taskIndex].getSensorTypeLong();
  }
  return 0u;
}

void UserVarStruct::setSensorTypeLong(taskIndex_t taskIndex, unsigned long value)
{
  if (taskIndex < _data.size()) {
    _data[taskIndex].setSensorTypeLong(value);
  }
}

#if FEATURE_EXTENDED_TASK_VALUE_TYPES

int32_t UserVarStruct::getInt32(taskIndex_t taskIndex,
                                uint8_t     varNr) const
{
  if (taskIndex < _data.size()) {
    return _data[taskIndex].getInt32(varNr);
  }
  return 0;
}

void UserVarStruct::setInt32(taskIndex_t taskIndex,
                             uint8_t     varNr,
                             int32_t     value)
{
  if (taskIndex < _data.size()) {
    _data[taskIndex].setInt32(varNr, value);
  }
}

#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

uint32_t UserVarStruct::getUint32(taskIndex_t taskIndex, uint8_t varNr) const
{
  if (taskIndex < _data.size()) {
    return _data[taskIndex].getUint32(varNr);
  }
  return 0u;
}

void UserVarStruct::setUint32(taskIndex_t taskIndex, uint8_t varNr, uint32_t value)
{
  if (taskIndex < _data.size()) {
    _data[taskIndex].setUint32(varNr, value);
  }
}

#if FEATURE_EXTENDED_TASK_VALUE_TYPES

int64_t UserVarStruct::getInt64(taskIndex_t taskIndex,
                                uint8_t     varNr) const
{
  if (taskIndex < _data.size()) {
    return _data[taskIndex].getInt64(varNr);
  }
  return 0;
}

void UserVarStruct::setInt64(taskIndex_t taskIndex,
                             uint8_t     varNr,
                             int64_t     value)
{
  if (taskIndex < _data.size()) {
    _data[taskIndex].setInt64(varNr, value);
  }
}

uint64_t UserVarStruct::getUint64(taskIndex_t taskIndex,
                                  uint8_t     varNr) const
{
  if (taskIndex < _data.size()) {
    return _data[taskIndex].getUint64(varNr);
  }
  return 0u;
}

void UserVarStruct::setUint64(taskIndex_t taskIndex,
                              uint8_t     varNr,
                              uint64_t    value)
{
  if (taskIndex < _data.size()) {
    _data[taskIndex].setUint64(varNr, value);
  }
}

#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

float UserVarStruct::getFloat(taskIndex_t taskIndex,
                              uint8_t     varNr) const
{
  if (taskIndex < _data.size()) {
    return _data[taskIndex].getFloat(varNr);
  }
  return 0.0f;
}

void UserVarStruct::setFloat(taskIndex_t taskIndex,
                             uint8_t     varNr,
                             float       value)
{
  if (taskIndex < _data.size()) {
    _data[taskIndex].setFloat(varNr, value);
  }
}

#if FEATURE_EXTENDED_TASK_VALUE_TYPES
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
double UserVarStruct::getDouble(taskIndex_t taskIndex,
                                uint8_t     varNr) const
{
  if (taskIndex < _data.size()) {
    return _data[taskIndex].getDouble(varNr);
  }
  return 0.0;
}

void UserVarStruct::setDouble(taskIndex_t taskIndex,
                              uint8_t     varNr,
                              double      value)
{
  if (taskIndex < _data.size()) {
    _data[taskIndex].setDouble(varNr, value);
  }
}
#endif
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

ESPEASY_RULES_FLOAT_TYPE UserVarStruct::getAsDouble(taskIndex_t  taskIndex,
                                  uint8_t      varNr,
                                  Sensor_VType sensorType) const
{
  if (taskIndex < _data.size()) {
    return _data[taskIndex].getAsDouble(varNr, sensorType);
  }
  return 0.0;
}

String UserVarStruct::getAsString(taskIndex_t taskIndex, uint8_t varNr, Sensor_VType  sensorType, uint8_t nrDecimals) const
{
  if (taskIndex < _data.size()) {
    return _data[taskIndex].getAsString(varNr, sensorType, nrDecimals);
  }
  return EMPTY_STRING;
}

void UserVarStruct::set(taskIndex_t taskIndex, uint8_t varNr, const ESPEASY_RULES_FLOAT_TYPE& value, Sensor_VType sensorType)
{
  if (taskIndex < _data.size()) {
    _data[taskIndex].set(varNr, value, sensorType);
  }
}

bool UserVarStruct::isValid(taskIndex_t  taskIndex,
                            uint8_t      varNr,
                            Sensor_VType sensorType) const
{
  if (taskIndex < _data.size()) {
    return _data[taskIndex].isValid(varNr, sensorType);
  }
  return false;
}

uint8_t * UserVarStruct::get(size_t& sizeInBytes)
{
  sizeInBytes = _data.size() * sizeof(TaskValues_Data_t);
  return reinterpret_cast<uint8_t *>(&_data[0]);
}

const TaskValues_Data_t * UserVarStruct::getTaskValues_Data(taskIndex_t taskIndex) const
{
  if (taskIndex < _data.size()) {
    return &_data[taskIndex];
  }
  return nullptr;
}

TaskValues_Data_t * UserVarStruct::getTaskValues_Data(taskIndex_t taskIndex)
{
  if (taskIndex < _data.size()) {
    return &_data[taskIndex];
  }
  return nullptr;
}

uint32_t UserVarStruct::compute_CRC32() const
{
  const uint8_t *buffer = reinterpret_cast<const uint8_t *>(&_data[0]);
  const size_t   size   = _data.size() * sizeof(TaskValues_Data_t);

  return calc_CRC32(buffer, size);
}
