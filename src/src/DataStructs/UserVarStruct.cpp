#include "../DataStructs/UserVarStruct.h"

#include "../DataStructs/TimingStats.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Cache.h"
#include "../Globals/Plugins.h"
#include "../Globals/RulesCalculate.h"
#include "../Helpers/_Plugin_SensorTypeHelper.h"
#include "../Helpers/CRC_functions.h"
#include "../Helpers/StringParser.h"



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

/*
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
 */
float UserVarStruct::operator[](unsigned int index) const
{
  const unsigned int taskIndex = index / VARS_PER_TASK;
  const unsigned int varNr     = index % VARS_PER_TASK;

  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr);

  if (data != nullptr) {
    return data->getFloat(varNr);
  } else {
    static float errorvalue = NAN;
    addLog(LOG_LEVEL_ERROR, F("UserVar index out of range"));
    return errorvalue;
  }
}

unsigned long UserVarStruct::getSensorTypeLong(taskIndex_t taskIndex) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, 0);

  if (data != nullptr) {
    return data->getSensorTypeLong();
  }
  return 0u;
}

void UserVarStruct::setSensorTypeLong(taskIndex_t taskIndex, unsigned long value)
{
  if (taskIndex < _data.size()) {
    if (Cache.hasFormula(taskIndex, 0)) {
      const ESPEASY_RULES_FLOAT_TYPE tmp = value;
      applyFormula(taskIndex, 0, tmp, Sensor_VType::SENSOR_TYPE_ULONG);
    } else {
      _data[taskIndex].setSensorTypeLong(value);
    }
  }
}

#if FEATURE_EXTENDED_TASK_VALUE_TYPES

int32_t UserVarStruct::getInt32(taskIndex_t taskIndex,
                                uint8_t     varNr) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr);

  if (data != nullptr) {
    return data->getInt32(varNr);
  }
  return 0;
}

void UserVarStruct::setInt32(taskIndex_t taskIndex,
                             uint8_t     varNr,
                             int32_t     value)
{
  if (taskIndex < _data.size()) {
    if (Cache.hasFormula(taskIndex, varNr)) {
      const ESPEASY_RULES_FLOAT_TYPE tmp = value;
      applyFormula(taskIndex, varNr, tmp, Sensor_VType::SENSOR_TYPE_INT32_QUAD);
    } else {
      _data[taskIndex].setInt32(varNr, value);
    }
  }
}

#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

uint32_t UserVarStruct::getUint32(taskIndex_t taskIndex, uint8_t varNr) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr);

  if (data != nullptr) {
    return data->getUint32(varNr);
  }
  return 0u;
}

void UserVarStruct::setUint32(taskIndex_t taskIndex, uint8_t varNr, uint32_t value)
{
  if (taskIndex < _data.size()) {
    if (Cache.hasFormula(taskIndex, varNr)) {
      const ESPEASY_RULES_FLOAT_TYPE tmp = value;
      applyFormula(taskIndex, varNr, tmp, Sensor_VType::SENSOR_TYPE_UINT32_QUAD);
    } else {
      _data[taskIndex].setUint32(varNr, value);
    }
  }
}

#if FEATURE_EXTENDED_TASK_VALUE_TYPES

int64_t UserVarStruct::getInt64(taskIndex_t taskIndex,
                                uint8_t     varNr) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr);

  if (data != nullptr) {
    return data->getInt64(varNr);
  }
  return 0;
}

void UserVarStruct::setInt64(taskIndex_t taskIndex,
                             uint8_t     varNr,
                             int64_t     value)
{
  if (taskIndex < _data.size()) {
    if (Cache.hasFormula(taskIndex, varNr)) {
      const ESPEASY_RULES_FLOAT_TYPE tmp = value;
      applyFormula(taskIndex, varNr, tmp, Sensor_VType::SENSOR_TYPE_INT64_DUAL);
    } else {
      _data[taskIndex].setInt64(varNr, value);
    }
  }
}

uint64_t UserVarStruct::getUint64(taskIndex_t taskIndex,
                                  uint8_t     varNr) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr);

  if (data != nullptr) {
    return data->getUint64(varNr);
  }
  return 0u;
}

void UserVarStruct::setUint64(taskIndex_t taskIndex,
                              uint8_t     varNr,
                              uint64_t    value)
{
  if (taskIndex < _data.size()) {
    if (Cache.hasFormula(taskIndex, varNr)) {
      const ESPEASY_RULES_FLOAT_TYPE tmp = value;
      applyFormula(taskIndex, varNr, tmp, Sensor_VType::SENSOR_TYPE_UINT64_DUAL);
    } else {
      _data[taskIndex].setUint64(varNr, value);
    }
  }
}

#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

float UserVarStruct::getFloat(taskIndex_t taskIndex,
                              uint8_t     varNr) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr);

  if (data != nullptr) {
    return data->getFloat(varNr);
  }
  return 0.0f;
}

void UserVarStruct::setFloat(taskIndex_t taskIndex,
                             uint8_t     varNr,
                             float       value)
{
  if (taskIndex < _data.size()) {
    if (Cache.hasFormula(taskIndex, varNr)) {
      const ESPEASY_RULES_FLOAT_TYPE tmp = value;
      applyFormula(taskIndex, varNr, tmp, Sensor_VType::SENSOR_TYPE_QUAD);
    } else {
      _data[taskIndex].setFloat(varNr, value);
    }
  }
}

#if FEATURE_EXTENDED_TASK_VALUE_TYPES
# if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
double UserVarStruct::getDouble(taskIndex_t taskIndex,
                                uint8_t     varNr) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr);

  if (data != nullptr) {
    return data->getDouble(varNr);
  }
  return 0.0;
}

void UserVarStruct::setDouble(taskIndex_t taskIndex,
                              uint8_t     varNr,
                              double      value)
{
  if (taskIndex < _data.size()) {
    if (Cache.hasFormula(taskIndex, varNr)) {
      applyFormula(taskIndex, varNr, value, Sensor_VType::SENSOR_TYPE_DOUBLE_DUAL);
    } else {
      _data[taskIndex].setDouble(varNr, value);
    }
  }
}

# endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
#endif  // if FEATURE_EXTENDED_TASK_VALUE_TYPES

ESPEASY_RULES_FLOAT_TYPE UserVarStruct::getAsDouble(taskIndex_t  taskIndex,
                                                    uint8_t      varNr,
                                                    Sensor_VType sensorType) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr);

  if (data != nullptr) {
    return data->getAsDouble(varNr, sensorType);
  }
  return 0.0;
}

String UserVarStruct::getAsString(taskIndex_t taskIndex, uint8_t varNr, Sensor_VType  sensorType, uint8_t nrDecimals) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr);

  if (data != nullptr) {
    return data->getAsString(varNr, sensorType, nrDecimals);
  }
  return EMPTY_STRING;
}

void UserVarStruct::set(taskIndex_t taskIndex, uint8_t varNr, const ESPEASY_RULES_FLOAT_TYPE& value, Sensor_VType sensorType)
{
  applyFormula(taskIndex, varNr, value, sensorType);
}

bool UserVarStruct::isValid(taskIndex_t  taskIndex,
                            uint8_t      varNr,
                            Sensor_VType sensorType) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr);

  if (data != nullptr) {
    return data->isValid(varNr, sensorType);
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

void UserVarStruct::clear_computed(taskIndex_t taskIndex)
{
  if (!Cache.hasFormula(taskIndex)) {
    auto it = _computed.find(taskIndex);

    if (it != _computed.end()) {
      _computed.erase(it);
    }
  }
}

const TaskValues_Data_t * UserVarStruct::getRawOrComputed(taskIndex_t taskIndex, uint8_t varNr) const
{
  if (Cache.hasFormula(taskIndex, varNr)) {
    auto it = _computed.find(taskIndex);

    if (it != _computed.end()) {
      return &(it->second);
    }
  }
  return getTaskValues_Data(taskIndex);
}

void UserVarStruct::applyFormula(taskIndex_t                     taskIndex,
                                 uint8_t                         varNr,
                                 const ESPEASY_RULES_FLOAT_TYPE& value,
                                 Sensor_VType                    sensorType)
{
  if ((taskIndex >= _data.size()) || (varNr >= VARS_PER_TASK)) {
    return;
  }
  String formula = Cache.getTaskDeviceFormula(taskIndex, varNr);

  if (!formula.isEmpty())
  {
    START_TIMER;

    // TD-er: Should we use the set nr of decimals here, or not round at all?
    // See: https://github.com/letscontrolit/ESPEasy/issues/3721#issuecomment-889649437
    const int nrDecimals = Cache.getTaskDeviceValueDecimals(taskIndex, varNr);

    if (formula.indexOf(F("%pvalue%")) != -1) {
      formula.replace(F("%pvalue%"), getAsString(taskIndex, varNr, sensorType, nrDecimals));
    }

    if (formula.indexOf(F("%value%")) != -1)
    {
      // Use a temporary TaskValues_Data_t object to have uniform formatting
      TaskValues_Data_t tmp;
      tmp.set(varNr, value, sensorType);
      formula.replace(F("%value%"), tmp.getAsString(varNr, sensorType, nrDecimals));
    }

    ESPEASY_RULES_FLOAT_TYPE result{};

    if (!isError(Calculate(parseTemplate(formula), result))) {
      _computed[taskIndex].set(varNr, result, sensorType);
    } else {
      // FIXME TD-er: What to do now? Just copy the raw value, set error value or don't update?
    }

    STOP_TIMER(COMPUTE_FORMULA_STATS);
  }

  _data[taskIndex].set(varNr, value, sensorType);
}
