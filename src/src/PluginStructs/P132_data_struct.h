#ifndef PLUGINSTRUCTS_P132_DATA_STRUCT_H
#define PLUGINSTRUCTS_P132_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P132

# ifndef P132_EXTENDED
#  ifdef ESP8266
#   define P132_EXTENDED  0
#  endif // ifdef ESP8266
#  ifdef ESP32
#   define P132_EXTENDED  1
#  endif // ifdef ESP32
# endif // ifndef P132_EXTENDED

// # define P132_DEBUG_LOG // Enable for some (extra) logging

# if P132_EXTENDED
#  define PLUGIN_NAME_132       "Energy (DC) - INA3221/INA226/INA228/INA231/INA260"
# else // if P132_EXTENDED
#  define PLUGIN_NAME_132       "Energy (DC) - INA3221"
# endif // if P132_EXTENDED

# define P132_CONFIG_BASE       2 // Better not change this...
# define P132_INA_TYPE          PCONFIG(0)
# define P132_INA_PREVIOUS      PCONFIG(7)
# define P132_I2C_ADDR          PCONFIG(1)
# define P132_VALUE_1           PCONFIG(P132_CONFIG_BASE)
# define P132_VALUE_2           PCONFIG(P132_CONFIG_BASE + 1)
# define P132_VALUE_3           PCONFIG(P132_CONFIG_BASE + 2)
# define P132_VALUE_4           PCONFIG(P132_CONFIG_BASE + 3)
# define P132_SHUNT             PCONFIG(6)
# define P132_MAX_CURRENT       PCONFIG_LONG(1)
# define P132_SHUNT_V2          PCONFIG_ULONG(2)

# define P132_CFG_VERSION       0x1    // Config version: 0 = V1, 1 = V2

# define P132_CONFIG_FLAGS          PCONFIG_ULONG(0)
# define P132_FLAG_AVERAGE          0  // 3 bits
# define P132_FLAG_CONVERSION_B     3  // 3 bits, V1 config
# define P132_FLAG_CONVERSION_S     6  // 3 bits, V1 config
# define P132_FLAG_CFG_VERSION      9  // 2 bits
# define P132_FLAG_V2_CONVERSION_B  11 // 4 bits, V2 config
# define P132_FLAG_V2_CONVERSION_S  15 // 4 bits, V2 config

# define P132_SET_AVERAGE(S) set3BitToUL(P132_CONFIG_FLAGS, P132_FLAG_AVERAGE, (S))
# define P132_GET_AVERAGE    get3BitFromUL(P132_CONFIG_FLAGS, P132_FLAG_AVERAGE)
# define P132_SET_CONVERSION_B(S) set3BitToUL(P132_CONFIG_FLAGS, P132_FLAG_CONVERSION_B, (S))
# define P132_GET_CONVERSION_B    get3BitFromUL(P132_CONFIG_FLAGS, P132_FLAG_CONVERSION_B)
# define P132_SET_CONVERSION_S(S) set3BitToUL(P132_CONFIG_FLAGS, P132_FLAG_CONVERSION_S, (S))
# define P132_GET_CONVERSION_S    get3BitFromUL(P132_CONFIG_FLAGS, P132_FLAG_CONVERSION_S)
# define P132_SET_CFG_VERSION(S) set2BitToUL(P132_CONFIG_FLAGS, P132_FLAG_CFG_VERSION, (S))
# define P132_GET_CFG_VERSION    get2BitFromUL(P132_CONFIG_FLAGS, P132_FLAG_CFG_VERSION)
# define P132_SET_V2_CONVERSION_B(S) set4BitToUL(P132_CONFIG_FLAGS, P132_FLAG_V2_CONVERSION_B, (S))
# define P132_GET_V2_CONVERSION_B    get4BitFromUL(P132_CONFIG_FLAGS, P132_FLAG_V2_CONVERSION_B)
# define P132_SET_V2_CONVERSION_S(S) set4BitToUL(P132_CONFIG_FLAGS, P132_FLAG_V2_CONVERSION_S, (S))
# define P132_GET_V2_CONVERSION_S    get4BitFromUL(P132_CONFIG_FLAGS, P132_FLAG_V2_CONVERSION_S)

# define INA3221_AVERAGE_BIT          9
# define INA3221_CONVERSION_BUS_BIT   6
# define INA3221_CONVERSION_SHUNT_BIT 3

# if FEATURE_MQTT_DISCOVER
int Plugin_132_QueryVType(uint8_t value_nr);
# endif // if FEATURE_MQTT_DISCOVER

# if P132_EXTENDED
#  include <INA.h>

enum class P132_DeviceType : uint8_t {
  Ina3221    = 0u,
  Ina219     = 1u,
  Ina226     = 2u,
  Ina228     = 3u,
  Ina230     = 4u,
  Ina231     = 5u,
  Ina260     = 6u,
  InaUnknown = 255u,
};

const __FlashStringHelper* toString(P132_DeviceType deviceType);
const uint8_t              P132_DeviceTypeToINAType(P132_DeviceType deviceType);
const uint8_t              P132_DeviceTypeToMaxValues(P132_DeviceType deviceType);
# endif // if P132_EXTENDED

struct P132_data_struct : public PluginTaskData_base {
public:

  P132_data_struct(struct EventStruct *event);

  P132_data_struct() = delete;
  # if P132_EXTENDED
  virtual ~P132_data_struct();
  # else // if P132_EXTENDED
  virtual ~P132_data_struct() = default;
  # endif // if P132_EXTENDED

  float getShuntVoltage_mV(uint8_t reg);
  float getBusVoltage_V(uint8_t reg);
  # if P132_EXTENDED
  float getBusCurrent_mA(uint8_t reg);
  float getBusPower_mW(uint8_t reg);

  void  setCalibration(struct EventStruct *event);

  bool  isInitialized() const {
    return nullptr != INA;
  }

  # else // if P132_EXTENDED
  void setCalibration_INA3221(struct EventStruct *event);
  # endif // if P132_EXTENDED

private:

  # if P132_EXTENDED
  INA_Class      *INA = nullptr;
  P132_DeviceType _deviceType = P132_DeviceType::Ina3221; // Old default
  uint8_t         _device     = 0xFF;


  uint32_t getAverageBitsToFactor(uint8_t         bits,
                                  P132_DeviceType deviceType);
  uint32_t getConversionBitsToFactor(uint8_t         bits,
                                     P132_DeviceType deviceType);
  # else // if P132_EXTENDED
  int16_t  getBusVoltage_raw(byte reg);
  int16_t  getShuntVoltage_raw(byte reg);
  # endif // if P132_EXTENDED

  int8_t _i2c_address;
};
#endif // ifdef USES_P132
#endif // ifndef PLUGINSTRUCTS_P132_DATA_STRUCT_H
