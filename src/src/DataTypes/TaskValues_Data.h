#ifndef DATATYPES_TASKVALUES_DATA_H
#define DATATYPES_TASKVALUES_DATA_H

#include "../../ESPEasy_common.h"

#include "../DataTypes/SensorVType.h"

struct TaskValues_Data_t {
  TaskValues_Data_t();

  TaskValues_Data_t(const TaskValues_Data_t& other);

  TaskValues_Data_t& operator=(const TaskValues_Data_t& other);

  void clear();

  void copyValue(const TaskValues_Data_t& other, uint8_t varNr, Sensor_VType  sensorType);

  unsigned long getSensorTypeLong() const;
  void          setSensorTypeLong(unsigned long value);

#if FEATURE_EXTENDED_TASK_VALUE_TYPES
  int32_t       getInt32(uint8_t varNr) const;
  void          setInt32(uint8_t varNr,
                         int32_t value);
#endif

  uint32_t      getUint32(uint8_t varNr) const;
  void          setUint32(uint8_t  varNr,
                          uint32_t value);

#if FEATURE_EXTENDED_TASK_VALUE_TYPES

  int64_t  getInt64(uint8_t varNr) const;
  void     setInt64(uint8_t varNr,
                    int64_t value);

  uint64_t getUint64(uint8_t varNr) const;
  void     setUint64(uint8_t  varNr,
                     uint64_t value);
#endif

  float    getFloat(uint8_t varNr) const;
  void     setFloat(uint8_t varNr,
                    float   value);

#if FEATURE_EXTENDED_TASK_VALUE_TYPES

  double getDouble(uint8_t varNr) const;
  void   setDouble(uint8_t varNr,
                   double  value);
#endif

  // Interpret the data according to the given sensorType
  double getAsDouble(uint8_t      varNr,
                     Sensor_VType sensorType) const;

  void   set(uint8_t       varNr,
             const double& value,
             Sensor_VType  sensorType);

  bool isValid(uint8_t       varNr,
               Sensor_VType  sensorType) const;

  String getAsString(uint8_t varNr, Sensor_VType  sensorType, uint8_t nrDecimals = 0) const;


  union {
    uint8_t  binary[VARS_PER_TASK * sizeof(float)];
    float    floats[VARS_PER_TASK];
    uint32_t uint32s[VARS_PER_TASK];
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
    int32_t  int32s[VARS_PER_TASK];
    uint64_t uint64s[VARS_PER_TASK / 2];
    int64_t  int64s[VARS_PER_TASK / 2];
    double   doubles[VARS_PER_TASK / 2];
#endif
  };
};

#endif // ifndef DATATYPES_TASKVALUES_DATA_H
