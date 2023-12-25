#include "../DataStructs/UserVarStruct.h"

#include "../DataStructs/TimingStats.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Cache.h"
#include "../Globals/Plugins.h"
#include "../Globals/RulesCalculate.h"
#include "../Helpers/_Plugin_SensorTypeHelper.h"
#include "../Helpers/CRC_functions.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"



void UserVarStruct::clear()
{
  for (size_t i = 0; i < TASKS_MAX; ++i) {
    _rawData[i].clear();
  }
  _computed.clear();
#ifndef LIMIT_BUILD_SIZE
  _preprocessedFormula.clear();
#endif // ifndef LIMIT_BUILD_SIZE
  _prevValue.clear();
}

float UserVarStruct::operator[](unsigned int index) const
{
  const unsigned int taskIndex = index / VARS_PER_TASK;
  const unsigned int varNr     = index % VARS_PER_TASK;

  constexpr bool raw = false;

  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr, Sensor_VType::SENSOR_TYPE_QUAD, raw);

  if (data != nullptr) {
    return data->getFloat(varNr);
  } else {
    static float errorvalue = NAN;
#ifndef LIMIT_BUILD_SIZE
    addLog(LOG_LEVEL_ERROR, F("UserVar index out of range"));
#endif
    return errorvalue;
  }
}

unsigned long UserVarStruct::getSensorTypeLong(taskIndex_t taskIndex, bool raw) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, 0, Sensor_VType::SENSOR_TYPE_ULONG, raw);

  if (data != nullptr) {
    return data->getSensorTypeLong();
  }
  return 0u;
}

void UserVarStruct::setSensorTypeLong(taskIndex_t taskIndex, unsigned long value)
{
  if (validTaskIndex(taskIndex)) {
    if (Cache.hasFormula(taskIndex, 0)) {
      const ESPEASY_RULES_FLOAT_TYPE tmp = value;
      applyFormulaAndSet(taskIndex, 0, tmp, Sensor_VType::SENSOR_TYPE_ULONG);
    } else {
      _rawData[taskIndex].setSensorTypeLong(value);
    }
  }
}

#if FEATURE_EXTENDED_TASK_VALUE_TYPES

int32_t UserVarStruct::getInt32(taskIndex_t    taskIndex,
                                taskVarIndex_t varNr,
                                bool           raw) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr, Sensor_VType::SENSOR_TYPE_INT32_QUAD, raw);

  if (data != nullptr) {
    return data->getInt32(varNr);
  }
  return 0;
}

void UserVarStruct::setInt32(taskIndex_t    taskIndex,
                             taskVarIndex_t varNr,
                             int32_t        value)
{
  if (validTaskIndex(taskIndex)) {
    if (Cache.hasFormula(taskIndex, varNr)) {
      const ESPEASY_RULES_FLOAT_TYPE tmp = value;
      applyFormulaAndSet(taskIndex, varNr, tmp, Sensor_VType::SENSOR_TYPE_INT32_QUAD);
    } else {
      _rawData[taskIndex].setInt32(varNr, value);
    }
  }
}

#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

uint32_t UserVarStruct::getUint32(taskIndex_t taskIndex, taskVarIndex_t varNr, bool raw) const
{
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr, Sensor_VType::SENSOR_TYPE_UINT32_QUAD, raw);
#else // if FEATURE_EXTENDED_TASK_VALUE_TYPES
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr, Sensor_VType::SENSOR_TYPE_NOT_SET, true);
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

  if (data != nullptr) {
    return data->getUint32(varNr);
  }
  return 0u;
}

void UserVarStruct::setUint32(taskIndex_t taskIndex, taskVarIndex_t varNr, uint32_t value)
{
  if (validTaskIndex(taskIndex)) {
    // setUInt32 is used to read taskvalues back from RTC
    // If FEATURE_EXTENDED_TASK_VALUE_TYPES is not enabled, this function will never be used for anything else
#if FEATURE_EXTENDED_TASK_VALUE_TYPES

    if (Cache.hasFormula(taskIndex, varNr)) {
      const ESPEASY_RULES_FLOAT_TYPE tmp = value;
      applyFormulaAndSet(taskIndex, varNr, tmp, Sensor_VType::SENSOR_TYPE_UINT32_QUAD);
    } else
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
    {
      _rawData[taskIndex].setUint32(varNr, value);
    }
  }
}

#if FEATURE_EXTENDED_TASK_VALUE_TYPES

int64_t UserVarStruct::getInt64(taskIndex_t    taskIndex,
                                taskVarIndex_t varNr,
                                bool           raw) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr, Sensor_VType::SENSOR_TYPE_INT64_DUAL, raw);

  if (data != nullptr) {
    return data->getInt64(varNr);
  }
  return 0;
}

void UserVarStruct::setInt64(taskIndex_t    taskIndex,
                             taskVarIndex_t varNr,
                             int64_t        value)
{
  if (validTaskIndex(taskIndex)) {
    if (Cache.hasFormula(taskIndex, varNr)) {
      const ESPEASY_RULES_FLOAT_TYPE tmp = value;

      if (applyFormulaAndSet(taskIndex, varNr, tmp, Sensor_VType::SENSOR_TYPE_INT64_DUAL)) {
        // Apply anyway so we don't loose resolution in the raw value
        _rawData[taskIndex].setInt64(varNr, value);
      }
    } else {
      _rawData[taskIndex].setInt64(varNr, value);
    }
  }
}

uint64_t UserVarStruct::getUint64(taskIndex_t    taskIndex,
                                  taskVarIndex_t varNr,
                                  bool           raw) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr, Sensor_VType::SENSOR_TYPE_UINT64_DUAL, raw);

  if (data != nullptr) {
    return data->getUint64(varNr);
  }
  return 0u;
}

void UserVarStruct::setUint64(taskIndex_t    taskIndex,
                              taskVarIndex_t varNr,
                              uint64_t       value)
{
  if (validTaskIndex(taskIndex)) {
    if (Cache.hasFormula(taskIndex, varNr)) {
      const ESPEASY_RULES_FLOAT_TYPE tmp = value;

      if (applyFormulaAndSet(taskIndex, varNr, tmp, Sensor_VType::SENSOR_TYPE_UINT64_DUAL)) {
        // Apply anyway so we don't loose resolution in the raw value
        _rawData[taskIndex].setUint64(varNr, value);
      }
    } else {
      _rawData[taskIndex].setUint64(varNr, value);
    }
  }
}

#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

float UserVarStruct::getFloat(taskIndex_t    taskIndex,
                              taskVarIndex_t varNr,
                              bool           raw) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr, Sensor_VType::SENSOR_TYPE_QUAD, raw);

  if (data != nullptr) {
    return data->getFloat(varNr);
  }
  return 0.0f;
}

void UserVarStruct::setFloat(taskIndex_t    taskIndex,
                             taskVarIndex_t varNr,
                             float          value)
{
  if (validTaskIndex(taskIndex)) {
    if (Cache.hasFormula(taskIndex, varNr)) {
      const ESPEASY_RULES_FLOAT_TYPE tmp = value;
      applyFormulaAndSet(taskIndex, varNr, tmp, Sensor_VType::SENSOR_TYPE_QUAD);
    } else {
      _rawData[taskIndex].setFloat(varNr, value);
    }
  }
}

#if FEATURE_EXTENDED_TASK_VALUE_TYPES
# if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
double UserVarStruct::getDouble(taskIndex_t taskIndex,
                                taskVarIndex_t varNr, bool raw) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr, Sensor_VType::SENSOR_TYPE_DOUBLE_DUAL, raw);

  if (data != nullptr) {
    return data->getDouble(varNr);
  }
  return 0.0;
}

void UserVarStruct::setDouble(taskIndex_t    taskIndex,
                              taskVarIndex_t varNr,
                              double         value)
{
  if (validTaskIndex(taskIndex)) {
    if (Cache.hasFormula(taskIndex, varNr)) {
      applyFormulaAndSet(taskIndex, varNr, value, Sensor_VType::SENSOR_TYPE_DOUBLE_DUAL);
    } else {
      _rawData[taskIndex].setDouble(varNr, value);
    }
  }
}

# endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
#endif  // if FEATURE_EXTENDED_TASK_VALUE_TYPES

ESPEASY_RULES_FLOAT_TYPE UserVarStruct::getAsDouble(taskIndex_t    taskIndex,
                                                    taskVarIndex_t varNr,
                                                    Sensor_VType   sensorType,
                                                    bool           raw) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr, sensorType, raw);

  if (data != nullptr) {
    return data->getAsDouble(varNr, sensorType);
  }
  return 0.0;
}

String UserVarStruct::getAsString(taskIndex_t taskIndex, taskVarIndex_t varNr, Sensor_VType  sensorType, uint8_t nrDecimals, bool raw) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr, sensorType, raw);

  if (data != nullptr) {
    if (nrDecimals == 255) {
      // TD-er: Should we use the set nr of decimals here, or not round at all?
      // See: https://github.com/letscontrolit/ESPEasy/issues/3721#issuecomment-889649437
      nrDecimals = Cache.getTaskDeviceValueDecimals(taskIndex, varNr);
    }

    return data->getAsString(varNr, sensorType, nrDecimals);
  }
  return EMPTY_STRING;
}

void UserVarStruct::set(taskIndex_t taskIndex, taskVarIndex_t varNr, const ESPEASY_RULES_FLOAT_TYPE& value, Sensor_VType sensorType)
{
  applyFormulaAndSet(taskIndex, varNr, value, sensorType);
}

bool UserVarStruct::isValid(taskIndex_t    taskIndex,
                            taskVarIndex_t varNr,
                            Sensor_VType   sensorType,
                            bool           raw) const
{
  const TaskValues_Data_t *data = getRawOrComputed(taskIndex, varNr, sensorType, raw);

  if (data != nullptr) {
    return data->isValid(varNr, sensorType);
  }
  return false;
}

uint8_t * UserVarStruct::get(size_t& sizeInBytes)
{
  constexpr size_t size_rawData = TASKS_MAX * sizeof(TaskValues_Data_t);

  sizeInBytes = size_rawData;
  return reinterpret_cast<uint8_t *>(&_rawData[0]);
}

const TaskValues_Data_t * UserVarStruct::getRawTaskValues_Data(taskIndex_t taskIndex) const
{
  if (validTaskIndex(taskIndex)) {
    return &_rawData[taskIndex];
  }
  return nullptr;
}

TaskValues_Data_t * UserVarStruct::getRawTaskValues_Data(taskIndex_t taskIndex)
{
  if (validTaskIndex(taskIndex)) {
    return &_rawData[taskIndex];
  }
  return nullptr;
}

uint32_t UserVarStruct::compute_CRC32() const
{
  const uint8_t   *buffer       = reinterpret_cast<const uint8_t *>(&_rawData[0]);
  constexpr size_t size_rawData = TASKS_MAX * sizeof(TaskValues_Data_t);

  return calc_CRC32(buffer, size_rawData);
}

void UserVarStruct::clear_computed(taskIndex_t taskIndex)
{
  if (!Cache.hasFormula(taskIndex)) {
    auto it = _computed.find(taskIndex);

    if (it != _computed.end()) {
      _computed.erase(it);
    }
  }
#ifndef LIMIT_BUILD_SIZE

  for (taskVarIndex_t varNr = 0; validTaskVarIndex(varNr); ++varNr) {
    const uint16_t key = makeWord(taskIndex, varNr);
    auto it            = _preprocessedFormula.find(key);

    if (it != _preprocessedFormula.end()) {
      _preprocessedFormula.erase(it);
    }
  }
#endif // ifndef LIMIT_BUILD_SIZE
}

void UserVarStruct::markPluginRead(taskIndex_t taskIndex)
{
  for (taskVarIndex_t varNr = 0; validTaskVarIndex(varNr); ++varNr) {
    if (Cache.hasFormula_with_prevValue(taskIndex, varNr)) {
      const uint16_t key = makeWord(taskIndex, varNr);
      _prevValue[key] = formatUserVarNoCheck(taskIndex, varNr);
    }
  }
}

const TaskValues_Data_t * UserVarStruct::getRawOrComputed(
  taskIndex_t    taskIndex,
  taskVarIndex_t varNr,
  Sensor_VType   sensorType,
  bool           raw) const
{
  if (!raw && Cache.hasFormula(taskIndex, varNr)) {
    auto it = _computed.find(taskIndex);

    if ((it == _computed.end()) || !it->second.isSet(varNr)) {
      // Try to compute values which do have a formula but not yet a 'computed' value cached.
      // FIXME TD-er: This may yield unexpected results when formula contains references to %pvalue%
      const int nrDecimals = Cache.getTaskDeviceValueDecimals(taskIndex, varNr);
      const String value   = getAsString(taskIndex, varNr, sensorType, nrDecimals, true);

      constexpr bool applyNow = true;

      if (applyFormula(taskIndex, varNr, value, sensorType, applyNow)) {
        it = _computed.find(taskIndex);
      }
    }

    if (it != _computed.end()) {
      if (it->second.isSet(varNr)) {
        return &(it->second.values);
      }
    }
  }
  return getRawTaskValues_Data(taskIndex);
}

bool UserVarStruct::applyFormula(taskIndex_t    taskIndex,
                                 taskVarIndex_t varNr,
                                 const String & value,
                                 Sensor_VType   sensorType,
                                 bool           applyNow) const
{
  if (!validTaskIndex(taskIndex) ||
      !validTaskVarIndex(varNr) ||
      (sensorType == Sensor_VType::SENSOR_TYPE_NOT_SET))
  {
    return false;
  }

  if (!applyNow && !Cache.hasFormula_with_prevValue(taskIndex, varNr)) {
    // Must check whether we can delay calculations until it is read for the first time.
    auto it = _computed.find(taskIndex);

    if (it != _computed.end()) {
      // Make sure it will apply formula when the value is actually read
      it->second.clear(varNr);
    }
    return true;
  }


  String formula = getPreprocessedFormula(taskIndex, varNr);
  bool   res     = true;

  if (!formula.isEmpty())
  {
    START_TIMER;

    // TD-er: Should we use the set nr of decimals here, or not round at all?
    // See: https://github.com/letscontrolit/ESPEasy/issues/3721#issuecomment-889649437
    if (formula.indexOf(F("%pvalue%")) != -1) {
      const String prev_str = getPreviousValue(taskIndex, varNr, sensorType);
      formula.replace(F("%pvalue%"), prev_str.isEmpty() ? value : prev_str);
    }

    formula.replace(F("%value%"), value);

    ESPEASY_RULES_FLOAT_TYPE result{};

    if (!isError(Calculate_preProcessed(parseTemplate(formula), result))) {
      _computed[taskIndex].set(varNr, result, sensorType);
    } else {
      // FIXME TD-er: What to do now? Just copy the raw value, set error value or don't update?
      res = false;
    }

    STOP_TIMER(COMPUTE_FORMULA_STATS);
  }
  return res;
}

bool UserVarStruct::applyFormulaAndSet(taskIndex_t                     taskIndex,
                                       taskVarIndex_t                  varNr,
                                       const ESPEASY_RULES_FLOAT_TYPE& value,
                                       Sensor_VType                    sensorType)
{
  if (!Cache.hasFormula(taskIndex, varNr)) {
    _rawData[taskIndex].set(varNr, value, sensorType);
    return true;
  }

  // Use a temporary TaskValues_Data_t object to have uniform formatting
  TaskValues_Data_t tmp;

  tmp.set(varNr, value, sensorType);
  const uint8_t nrDecimals = Cache.getTaskDeviceValueDecimals(taskIndex, varNr);
  const String  value_str  = tmp.getAsString(varNr, sensorType, nrDecimals);

  constexpr bool applyNow = false;

  if (applyFormula(taskIndex, varNr, value_str, sensorType, applyNow)) {
    _rawData[taskIndex].set(varNr, value, sensorType);
    return true;
  }
  return false;
}

String UserVarStruct::getPreprocessedFormula(taskIndex_t taskIndex, taskVarIndex_t varNr) const
{
  if (!Cache.hasFormula(taskIndex, varNr)) {
    return EMPTY_STRING;
  }

#ifndef LIMIT_BUILD_SIZE
  const uint16_t key = makeWord(taskIndex, varNr);
  auto it            = _preprocessedFormula.find(key);

  if (it == _preprocessedFormula.end()) {
    _preprocessedFormula[key] = RulesCalculate_t::preProces(Cache.getTaskDeviceFormula(taskIndex, varNr));
  }
  return _preprocessedFormula[key];
#else // ifndef LIMIT_BUILD_SIZE
  return RulesCalculate_t::preProces(Cache.getTaskDeviceFormula(taskIndex, varNr));
#endif // ifndef LIMIT_BUILD_SIZE
}

String UserVarStruct::getPreviousValue(taskIndex_t taskIndex, taskVarIndex_t varNr, Sensor_VType sensorType) const
{
  /*
     if (!Cache.hasFormula_with_prevValue(taskIndex, varNr)) {
     // Should not happen.

     }
   */

  const uint16_t key = makeWord(taskIndex, varNr);
  auto it            = _prevValue.find(key);

  if (it != _prevValue.end()) {
    return it->second;
  }

  // Probably the first run, so just return the current value

  // Do not call getAsString here as this will result in stack overflow.
  return EMPTY_STRING;
}
