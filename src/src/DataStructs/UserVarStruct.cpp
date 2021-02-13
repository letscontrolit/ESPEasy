#include "../DataStructs/UserVarStruct.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Plugins.h"

UserVarStruct::UserVarStruct()
{
  _data.resize(VARS_PER_TASK * TASKS_MAX, 0.0f);
}

// Implementation of [] operator.  This function must return a
// reference as array element can be put on left side
float& UserVarStruct::operator[](unsigned int index)
{
  if (index >= _data.size()) {
    static float errorvalue = NAN;
    addLog(LOG_LEVEL_ERROR, F("UserVar index out of range"));
    return errorvalue;
  }
  return _data.at(index);
}

const float& UserVarStruct::operator[](unsigned int index) const
{
  if (index >= _data.size()) {
    static float errorvalue = NAN;
    addLog(LOG_LEVEL_ERROR, F("UserVar index out of range"));
    return errorvalue;
  }
  return _data.at(index);
}

unsigned long UserVarStruct::getSensorTypeLong(taskIndex_t taskIndex) const
{
  unsigned long value = 0;

  if (validTaskIndex(taskIndex)) {
    const unsigned int baseVarIndex = taskIndex * VARS_PER_TASK;
    value = static_cast<unsigned long>(_data[baseVarIndex]) + (static_cast<unsigned long>(_data[baseVarIndex + 1]) << 16);
  }
  return value;
}

void UserVarStruct::setSensorTypeLong(taskIndex_t taskIndex, unsigned long value)
{
  if (!validTaskIndex(taskIndex)) {
    return;
  }
  const unsigned int baseVarIndex = taskIndex * VARS_PER_TASK;

  _data[baseVarIndex]     = value & 0xFFFF;
  _data[baseVarIndex + 1] = (value >> 16) & 0xFFFF;
}

uint32_t UserVarStruct::getUint32(taskIndex_t taskIndex, byte varNr) const
{
  if (!validTaskIndex(taskIndex) || (varNr >= VARS_PER_TASK)) {
    addLog(LOG_LEVEL_ERROR, F("UserVar index out of range"));
    return 0;
  }
  const unsigned int baseVarIndex = taskIndex * VARS_PER_TASK;
  uint32_t res;
  memcpy(&res, &_data[baseVarIndex + varNr], sizeof(float));
  return res;
}

void UserVarStruct::setUint32(taskIndex_t taskIndex, byte varNr, uint32_t value)
{
  if (!validTaskIndex(taskIndex) || (varNr >= VARS_PER_TASK)) {
    addLog(LOG_LEVEL_ERROR, F("UserVar index out of range"));
    return;
  }
  const unsigned int baseVarIndex = taskIndex * VARS_PER_TASK;

  // Store in a new variable to prevent
  // warning: dereferencing type-punned pointer will break strict-aliasing rules [-Wstrict-aliasing]
  float tmp;
  memcpy(&tmp, &value, sizeof(float));
  _data[baseVarIndex + varNr] = tmp;
}

size_t UserVarStruct::getNrElements() const
{
  return _data.size();
}

byte * UserVarStruct::get()
{
  return (byte *)(&_data[0]);
}
