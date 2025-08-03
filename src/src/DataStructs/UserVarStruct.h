#ifndef DATASTRUCTS_USERVARSTRUCT_H
#define DATASTRUCTS_USERVARSTRUCT_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/DeviceStruct.h"

#include "../DataTypes/TaskIndex.h"
#include "../DataTypes/TaskValues_Data.h"

#include <vector>
#include <map>

struct TaskValues_Data_cache {
  void set(taskVarIndex_t                  varNr,
           const ESPEASY_RULES_FLOAT_TYPE& value,
           Sensor_VType                    sensorType)
  {
    values.set(varNr, value, sensorType);
    bitSet(values_set_map, varNr);
  }

  void clear(taskVarIndex_t varNr) {
    bitClear(values_set_map, varNr);
  }

  bool isSet(taskVarIndex_t varNr) const {
    return bitRead(values_set_map, varNr);
  }

  TaskValues_Data_t values{};
  uint32_t          values_set_map{};
};

struct UserVarStruct {
  UserVarStruct() = default;

  void          clear();

  float         operator[](unsigned int index) const;

  // Legacy "long" type, which was spread over several floats.
  unsigned long getSensorTypeLong(taskIndex_t taskIndex,
                                  bool        raw = false) const;
  void          setSensorTypeLong(taskIndex_t   taskIndex,
                                  unsigned long value);

#if FEATURE_EXTENDED_TASK_VALUE_TYPES

  // 32 bit signed int stored at the memory location of the float
  int32_t getInt32(taskIndex_t    taskIndex,
                   taskVarIndex_t varNr,
                   bool           raw = false) const;
  void    setInt32(taskIndex_t    taskIndex,
                   taskVarIndex_t varNr,
                   int32_t        value);
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

  // 32 bit unsigned int stored at the memory location of the float
  uint32_t getUint32(taskIndex_t    taskIndex,
                     taskVarIndex_t varNr,
                     bool           raw = false) const;
  void     setUint32(taskIndex_t    taskIndex,
                     taskVarIndex_t varNr,
                     uint32_t       value);

#if FEATURE_EXTENDED_TASK_VALUE_TYPES

  // 64 bit signed int stored at the memory location of the float
  int64_t getInt64(taskIndex_t    taskIndex,
                   taskVarIndex_t varNr,
                   bool           raw = false) const;
  void    setInt64(taskIndex_t    taskIndex,
                   taskVarIndex_t varNr,
                   int64_t        value);

  // 64 bit unsigned int stored at the memory location of the float
  uint64_t getUint64(taskIndex_t    taskIndex,
                     taskVarIndex_t varNr,
                     bool           raw = false) const;
  void     setUint64(taskIndex_t    taskIndex,
                     taskVarIndex_t varNr,
                     uint64_t       value);
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

  float getFloat(taskIndex_t    taskIndex,
                 taskVarIndex_t varNr,
                 bool           raw = false) const;
  void  setFloat(taskIndex_t    taskIndex,
                 taskVarIndex_t varNr,
                 float          value);

#if FEATURE_EXTENDED_TASK_VALUE_TYPES
# if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE

  // Double stored at the memory location of the float
  double getDouble(taskIndex_t    taskIndex,
                   taskVarIndex_t varNr,
                   bool           raw = false) const;
  void   setDouble(taskIndex_t    taskIndex,
                   taskVarIndex_t varNr,
                   double         value);
# endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
#endif  // if FEATURE_EXTENDED_TASK_VALUE_TYPES

  ESPEASY_RULES_FLOAT_TYPE getAsDouble(taskIndex_t    taskIndex,
                                       taskVarIndex_t varNr,
                                       Sensor_VType   sensorType,
                                       bool           raw = false) const;

  String getAsString(taskIndex_t    taskIndex,
                     taskVarIndex_t varNr,
                     Sensor_VType   sensorType,
                     uint8_t        nrDecimals = 255,
                     bool           raw        = false) const;


  void set(taskIndex_t                     taskIndex,
           taskVarIndex_t                  varNr,
           const ESPEASY_RULES_FLOAT_TYPE& value,
           Sensor_VType                    sensorType);

  bool isValid(taskIndex_t    taskIndex,
               taskVarIndex_t varNr,
               Sensor_VType   sensorType,
               bool           raw = false) const;

  uint8_t                * get(size_t& sizeInBytes);

  const TaskValues_Data_t* getRawTaskValues_Data(taskIndex_t taskIndex) const;
  TaskValues_Data_t      * getRawTaskValues_Data(taskIndex_t taskIndex);

  uint32_t                 compute_CRC32() const;

  void                     clear_computed(taskIndex_t taskIndex);

  void                     markPluginRead(taskIndex_t taskIndex);

private:

  const TaskValues_Data_t* getRawOrComputed(taskIndex_t    taskIndex,
                                            taskVarIndex_t varNr,
                                            Sensor_VType   sensorType,
                                            bool           raw) const;

  bool applyFormula(taskIndex_t    taskIndex,
                    taskVarIndex_t varNr,
                    const String & value,
                    Sensor_VType   sensorType,
                    bool           applyNow) const;

  bool applyFormulaAndSet(taskIndex_t                     taskIndex,
                          taskVarIndex_t                  varNr,
                          const ESPEASY_RULES_FLOAT_TYPE& value,
                          Sensor_VType                    sensorType);


  // Raw TaskValues data as stored in RTC
  TaskValues_Data_t _rawData[TASKS_MAX]{};

  // Computed TaskValues for those tasks which use a formula
  // Not stored in RTC, but used to cache calculated values.
  // Since we can refer to any previous value in a formula (%pvalue%),
  // we need to apply the formula when updating any value.
  mutable std::map<taskIndex_t, TaskValues_Data_cache>_computed;

  String getPreprocessedFormula(taskIndex_t    taskIndex,
                                taskVarIndex_t varNr) const;
public:
  String getPreviousValue(taskIndex_t    taskIndex,
                          taskVarIndex_t varNr,
                          Sensor_VType   sensorType) const;
private:
#ifndef LIMIT_BUILD_SIZE
  mutable std::map<uint16_t, String>_preprocessedFormula;
#endif // ifndef LIMIT_BUILD_SIZE
  mutable std::map<uint16_t, String>_prevValue;
};

#endif // ifndef DATASTRUCTS_USERVARSTRUCT_H
