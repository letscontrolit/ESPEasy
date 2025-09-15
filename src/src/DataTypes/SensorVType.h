#ifndef DATATYPES_SENSOR_VTYPE_H
#define DATATYPES_SENSOR_VTYPE_H

#include "../../ESPEasy_common.h"

# define Sensor_VType_CAN_SET 0x0100 // 8 bits of Sensor_VType and the 9th bit to enable /set

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

  SENSOR_TYPE_ANALOG_ONLY     =  100,
  SENSOR_TYPE_TEMP_ONLY       =  101,
  SENSOR_TYPE_HUM_ONLY        =  102,
  SENSOR_TYPE_LUX_ONLY        =  103,
  SENSOR_TYPE_DISTANCE_ONLY   =  104,
  SENSOR_TYPE_DIRECTION_ONLY  =  105,
  SENSOR_TYPE_DUSTPM2_5_ONLY  =  106,
  SENSOR_TYPE_DUSTPM1_0_ONLY  =  107,
  SENSOR_TYPE_DUSTPM10_ONLY   =  108,
  SENSOR_TYPE_MOISTURE_ONLY   =  109,
  SENSOR_TYPE_CO2_ONLY        =  110,
  SENSOR_TYPE_GPS_ONLY        =  111,
  SENSOR_TYPE_UV_ONLY         =  112,
  SENSOR_TYPE_UV_INDEX_ONLY   =  113,
  SENSOR_TYPE_IR_ONLY         =  114,
  SENSOR_TYPE_WEIGHT_ONLY     =  115,
  SENSOR_TYPE_VOLTAGE_ONLY    =  116,
  SENSOR_TYPE_CURRENT_ONLY    =  117,
  SENSOR_TYPE_POWER_USG_ONLY  =  118,
  SENSOR_TYPE_POWER_FACT_ONLY =  119,
  SENSOR_TYPE_APPRNT_POWER_USG_ONLY = 120,
  SENSOR_TYPE_TVOC_ONLY       =  121,
  SENSOR_TYPE_BARO_ONLY       =  122,
  SENSOR_TYPE_COLOR_RED_ONLY  =  123,
  SENSOR_TYPE_COLOR_GREEN_ONLY = 124,
  SENSOR_TYPE_COLOR_BLUE_ONLY =  125,
  SENSOR_TYPE_COLOR_TEMP_ONLY =  126,
  SENSOR_TYPE_REACTIVE_POWER_ONLY = 127,
  SENSOR_TYPE_AQI_ONLY        = 128,
  SENSOR_TYPE_NOX_ONLY        = 129,
  SENSOR_TYPE_SWITCH_INVERTED = 130,
  SENSOR_TYPE_WIND_SPEED      = 131,
  SENSOR_TYPE_DURATION        = 132,
  SENSOR_TYPE_DATE            = 133,
  SENSOR_TYPE_TIMESTAMP       = 134,
  SENSOR_TYPE_DATA_RATE       = 135,
  SENSOR_TYPE_DATA_SIZE       = 136,
  SENSOR_TYPE_SOUND_PRESSURE  = 137,
  SENSOR_TYPE_SIGNAL_STRENGTH = 138,
  SENSOR_TYPE_REACTIVE_ENERGY = 139,
  SENSOR_TYPE_FREQUENCY       = 140,
  SENSOR_TYPE_ENERGY          = 141,
  SENSOR_TYPE_ENERGY_STORAGE  = 142,
  SENSOR_TYPE_ABS_HUMIDITY    = 143,
  SENSOR_TYPE_ATMOS_PRESSURE  = 144,
  SENSOR_TYPE_BLOOD_GLUCOSE_C = 145,
  SENSOR_TYPE_CO_ONLY         = 146,
  SENSOR_TYPE_ENERGY_DISTANCE = 147,
  SENSOR_TYPE_GAS_ONLY        = 148,
  SENSOR_TYPE_NITROUS_OXIDE   = 149,
  SENSOR_TYPE_OZONE_ONLY      = 150,
  SENSOR_TYPE_PRECIPITATION   = 151,
  SENSOR_TYPE_PRECIPITATION_INTEN = 152,
  SENSOR_TYPE_SULPHUR_DIOXIDE = 153,
  SENSOR_TYPE_VOC_PARTS       = 154,
  SENSOR_TYPE_VOLUME          = 155,
  SENSOR_TYPE_VOLUME_FLOW_RATE = 156,
  SENSOR_TYPE_VOLUME_STORAGE  = 157,
  SENSOR_TYPE_WATER           = 158,

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
uint8_t getValueCountFromSensorType(Sensor_VType sensorType, bool log);

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

# if FEATURE_MQTT && FEATURE_MQTT_DISCOVER
String getValueType2HADeviceClass(Sensor_VType sensorType);
String getValueType2DefaultHAUoM(Sensor_VType sensorType);
# endif // if FEATURE_MQTT && FEATURE_MQTT_DISCOVER

#if FEATURE_CUSTOM_TASKVAR_VTYPE
bool isMQTTDiscoverySensorType(Sensor_VType sensorType);
#endif // if FEATURE_CUSTOM_TASKVAR_VTYPE


#endif