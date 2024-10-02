#ifndef DATATYPES_TASKVALUES_DATA_H
#define DATATYPES_TASKVALUES_DATA_H

#include "../../ESPEasy_common.h"

#include "../DataTypes/SensorVType.h"

struct alignas(uint32_t) TaskValues_Data_t {
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
#if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  double getDouble(uint8_t varNr) const;
  void   setDouble(uint8_t varNr,
                   double  value);
#endif
#endif

  // Interpret the data according to the given sensorType
  ESPEASY_RULES_FLOAT_TYPE getAsDouble(uint8_t      varNr,
                     Sensor_VType sensorType) const;

  void   set(uint8_t       varNr,
             const ESPEASY_RULES_FLOAT_TYPE& value,
             Sensor_VType  sensorType);

  bool isValid(uint8_t       varNr,
               Sensor_VType  sensorType) const;

  String getAsString(uint8_t varNr, Sensor_VType  sensorType, uint8_t nrDecimals = 0) const;

  uint8_t  binary[VARS_PER_TASK * sizeof(float)]{};
};

#endif // ifndef DATATYPES_TASKVALUES_DATA_H
