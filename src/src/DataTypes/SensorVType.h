#ifndef DATATYPES_SENSOR_VTYPE_H
#define DATATYPES_SENSOR_VTYPE_H

#include "../../ESPEasy_common.h"

enum class Sensor_VType : uint8_t {
  SENSOR_TYPE_NONE            =    0,
  SENSOR_TYPE_SINGLE          =    1,
  SENSOR_TYPE_TEMP_HUM        =    2,
  SENSOR_TYPE_TEMP_BARO       =    3,
  SENSOR_TYPE_TEMP_HUM_BARO   =    4,
  SENSOR_TYPE_DUAL            =    5,
  SENSOR_TYPE_TRIPLE          =    6,
  SENSOR_TYPE_QUAD            =    7,
  SENSOR_TYPE_TEMP_EMPTY_BARO =    8,
  SENSOR_TYPE_SWITCH          =   10,
  SENSOR_TYPE_DIMMER          =   11,
  SENSOR_TYPE_WIND            =   21,
  SENSOR_TYPE_STRING          =   22,
  SENSOR_TYPE_ULONG           =   20,  // Was called SENSOR_TYPE_LONG, but actually it was an unsigned type
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
  SENSOR_TYPE_UINT32_DUAL     =   31,
  SENSOR_TYPE_UINT32_TRIPLE   =   32,
  SENSOR_TYPE_UINT32_QUAD     =   33,
  SENSOR_TYPE_INT32_SINGLE    =   40,
  SENSOR_TYPE_INT32_DUAL      =   41,
  SENSOR_TYPE_INT32_TRIPLE    =   42,
  SENSOR_TYPE_INT32_QUAD      =   43,
  SENSOR_TYPE_UINT64_SINGLE   =   50,
  SENSOR_TYPE_UINT64_DUAL     =   51,
  SENSOR_TYPE_INT64_SINGLE    =   60,
  SENSOR_TYPE_INT64_DUAL      =   61,
  SENSOR_TYPE_DOUBLE_SINGLE   =   70,
  SENSOR_TYPE_DOUBLE_DUAL     =   71,
#endif

  SENSOR_TYPE_NOT_SET = 255
};

enum class Output_Data_type_t : uint8_t {
  Default = 0,
  Simple, // SENSOR_TYPE_SINGLE, _DUAL, _TRIPLE, _QUAD
  All
};

/*********************************************************************************************\
   Get value count from sensor type

   Only use this function to determine nr of output values when changing output type of a task
   To get the actual output values for a task, use getValueCountForTask
\*********************************************************************************************/
uint8_t getValueCountFromSensorType(Sensor_VType sensorType);

const __FlashStringHelper * getSensorTypeLabel(Sensor_VType sensorType);

bool isSimpleOutputDataType(Sensor_VType sensorType);

bool isUInt32OutputDataType(Sensor_VType sensorType);
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
bool isInt32OutputDataType(Sensor_VType sensorType);


bool isUInt64OutputDataType(Sensor_VType sensorType);
bool isInt64OutputDataType(Sensor_VType sensorType);
#endif

bool isFloatOutputDataType(Sensor_VType sensorType);
#if FEATURE_EXTENDED_TASK_VALUE_TYPES
bool isDoubleOutputDataType(Sensor_VType sensorType);
#endif

// To simplify checking whether formatting using decimals is desired.
bool isIntegerOutputDataType(Sensor_VType sensorType);

bool is32bitOutputDataType(Sensor_VType sensorType);




#endif