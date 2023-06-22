#ifndef DATASTRUCTS_USERVARSTRUCT_H
#define DATASTRUCTS_USERVARSTRUCT_H

#include "../../ESPEasy_common.h"

#include "../DataStructs/DeviceStruct.h"

#include "../DataTypes/TaskIndex.h"
#include "../DataTypes/TaskValues_Data.h"

#include <vector>

struct UserVarStruct {
  UserVarStruct();

  void          clear();

  // Overloading [] operator to access elements in array style
  float       & operator[](unsigned int index);
  const float & operator[](unsigned int index) const;

  // Legacy "long" type, which was spread over several floats.
  unsigned long getSensorTypeLong(taskIndex_t taskIndex) const;
  void          setSensorTypeLong(taskIndex_t   taskIndex,
                                  unsigned long value);

#if FEATURE_EXTENDED_TASK_VALUE_TYPES

  // 32 bit signed int stored at the memory location of the float
  int32_t getInt32(taskIndex_t taskIndex,
                   uint8_t     varNr) const;
  void    setInt32(taskIndex_t taskIndex,
                   uint8_t     varNr,
                   int32_t     value);
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

  // 32 bit unsigned int stored at the memory location of the float
  uint32_t getUint32(taskIndex_t taskIndex,
                     uint8_t     varNr) const;
  void     setUint32(taskIndex_t taskIndex,
                     uint8_t     varNr,
                     uint32_t    value);

#if FEATURE_EXTENDED_TASK_VALUE_TYPES

  // 64 bit signed int stored at the memory location of the float
  int64_t getInt64(taskIndex_t taskIndex,
                   uint8_t     varNr) const;
  void    setInt64(taskIndex_t taskIndex,
                   uint8_t     varNr,
                   int64_t     value);

  // 64 bit unsigned int stored at the memory location of the float
  uint64_t getUint64(taskIndex_t taskIndex,
                     uint8_t     varNr) const;
  void     setUint64(taskIndex_t taskIndex,
                     uint8_t     varNr,
                     uint64_t    value);
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

  float getFloat(taskIndex_t taskIndex,
                 uint8_t     varNr) const;
  void  setFloat(taskIndex_t taskIndex,
                 uint8_t     varNr,
                 float       value);

#if FEATURE_EXTENDED_TASK_VALUE_TYPES
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  // Double stored at the memory location of the float
  double getDouble(taskIndex_t taskIndex,
                   uint8_t     varNr) const;
  void   setDouble(taskIndex_t taskIndex,
                   uint8_t     varNr,
                   double      value);
#endif
#endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES

  ESPEASY_RULES_FLOAT_TYPE getAsDouble(taskIndex_t  taskIndex,
                     uint8_t      varNr,
                     Sensor_VType sensorType) const;

  String getAsString(taskIndex_t  taskIndex,
                     uint8_t      varNr,
                     Sensor_VType sensorType,
                     uint8_t      nrDecimals = 0) const;


  void set(taskIndex_t   taskIndex,
           uint8_t       varNr,
           const ESPEASY_RULES_FLOAT_TYPE& value,
           Sensor_VType  sensorType);

  bool isValid(taskIndex_t  taskIndex,
               uint8_t      varNr,
               Sensor_VType sensorType) const;

  uint8_t                * get(size_t& sizeInBytes);

  const TaskValues_Data_t* getTaskValues_Data(taskIndex_t taskIndex) const;
  TaskValues_Data_t      * getTaskValues_Data(taskIndex_t taskIndex);

  uint32_t                 compute_CRC32() const;

private:

  std::vector<TaskValues_Data_t>_data;
};

#endif // ifndef DATASTRUCTS_USERVARSTRUCT_H
