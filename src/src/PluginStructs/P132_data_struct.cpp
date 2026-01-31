#include "../PluginStructs/P132_data_struct.h"

#ifdef USES_P132

# if FEATURE_MQTT_DISCOVER
int Plugin_132_QueryVType(uint8_t value_nr) {
  if (value_nr < 6u) {
    const bool odd = value_nr % 2;
    return static_cast<int>(odd ? Sensor_VType::SENSOR_TYPE_CURRENT_ONLY : Sensor_VType::SENSOR_TYPE_VOLTAGE_ONLY);
  }
  return static_cast<int>(Sensor_VType::SENSOR_TYPE_POWER_USG_ONLY);
}

# endif // if FEATURE_MQTT_DISCOVER

# if P132_EXTENDED
const __FlashStringHelper* toString(P132_DeviceType deviceType) {
  switch (deviceType) {
    case P132_DeviceType::Ina3221: return F("INA3221");
    case P132_DeviceType::Ina219: return F("INA219");
    case P132_DeviceType::Ina226: return F("INA226");
    case P132_DeviceType::Ina228: return F("INA228");
    case P132_DeviceType::Ina230: return F("INA230");
    case P132_DeviceType::Ina231: return F("INA231");
    case P132_DeviceType::Ina260: return F("INA260");
    case P132_DeviceType::InaUnknown: return F("");
  }
  return F("");
}

const uint8_t P132_DeviceTypeToINAType(P132_DeviceType deviceType) {
  switch (deviceType) {
    case P132_DeviceType::Ina3221: return INA3221_0;
    case P132_DeviceType::Ina219: return INA219;
    case P132_DeviceType::Ina226: return INA226;
    case P132_DeviceType::Ina228: return INA228;
    case P132_DeviceType::Ina230: return INA230;
    case P132_DeviceType::Ina231: return INA231;
    case P132_DeviceType::Ina260: return INA260;
    case P132_DeviceType::InaUnknown: return INA_UNKNOWN;
  }
  return INA_UNKNOWN;
}

const P132_DeviceType P132_INATypeToDeviceType(uint8_t inaType) {
  switch (inaType) {
    case INA219: return P132_DeviceType::Ina219;
    case INA226: return P132_DeviceType::Ina226;
    case INA228: return P132_DeviceType::Ina228;
    case INA230: return P132_DeviceType::Ina230;
    case INA231: return P132_DeviceType::Ina231;
    case INA260: return P132_DeviceType::Ina260;
    case INA3221_0:
    case INA3221_1:
    case INA3221_2: return P132_DeviceType::Ina3221;
  }
  return P132_DeviceType::Ina3221;
}

const uint8_t P132_DeviceTypeToMaxValues(P132_DeviceType deviceType) {
  switch (deviceType) {
    case P132_DeviceType::Ina3221: return 9; // Voltage 1..3/Current 1..3/Power 1..3
    case P132_DeviceType::Ina219:
    case P132_DeviceType::Ina226:
    case P132_DeviceType::Ina228:
    case P132_DeviceType::Ina230:
    case P132_DeviceType::Ina231:
    case P132_DeviceType::Ina260: return 3; // Voltage/Current/Power
    case P132_DeviceType::InaUnknown: return 0;
  }
  return 0;
}

# endif // if P132_EXTENDED

// **************************************************************************/
// Constructor
// **************************************************************************/
# if P132_EXTENDED
P132_data_struct::P132_data_struct(struct EventStruct *event) {
  P132_DeviceType altType = P132_DeviceType::InaUnknown;

  _deviceType = static_cast<P132_DeviceType>(P132_INA_TYPE);

  // INA226 and INA231 are interchangeable
  if (P132_DeviceType::Ina226 == _deviceType) { altType = P132_DeviceType::Ina231; }

  if (P132_DeviceType::Ina231 == _deviceType) { altType = P132_DeviceType::Ina226; }

  _i2c_address = P132_I2C_ADDR;
  INA          = new INA_Class(16); // Max 16 addresses can be used, trade-off: INA3221 uses 3 slots...

  if (nullptr == INA) {
    addLog(LOG_LEVEL_ERROR, F("INA Class initialization failed"));
  } else {
    uint16_t maxCurrent  = max((uint16_t)1u, (uint16_t)P132_MAX_CURRENT);
    const uint32_t shunt = P132_CFG_VERSION != P132_GET_CFG_VERSION ? (100 / P132_SHUNT) * 1000 : P132_SHUNT_V2;
    const uint8_t  count = INA->begin(maxCurrent, shunt);
    uint8_t search       = 0;

    while (search < count) {
      const P132_DeviceType foundType = P132_INATypeToDeviceType(INA->getDeviceType(search));
      const uint8_t addr              = INA->getDeviceAddress(search);

      #  ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, strformat(F("INA  : Detected: %s at 0x%x, device index: %d"),
                                         INA->getDeviceName(search), INA->getDeviceAddress(search), search));
      }
      #  endif// ifndef BUILD_NO_DEBUG

      if (((foundType == _deviceType) || (foundType == altType)) && (addr == _i2c_address)) {
        _device = search;
        break; // Found, done searching
      }

      if (P132_DeviceType::Ina3221 == foundType) {
        search += 2; // Skip 'sub'-devices for INA3321
      }
      ++search;
    }

    if (0xFF == _device) {
      delete INA;
      INA = nullptr;
      addLog(LOG_LEVEL_ERROR, strformat(F("INA  : Configured %s at 0x%x not found."),
                                        FsP(toString(_deviceType)), _i2c_address));
    } else
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, strformat(F("INA  : Found: %s at 0x%x, device index: %d"),
                                       INA->getDeviceName(_device), INA->getDeviceAddress(_device), _device));
    }
  }
  setCalibration(event);
}

# else // if P132_EXTENDED
P132_data_struct::P132_data_struct(struct EventStruct *event) {
  _i2c_address = P132_I2C_ADDR;
  setCalibration_INA3221(event);
}

# endif // if P132_EXTENDED

// **************************************************************************/
// Destructor
// **************************************************************************/
# if P132_EXTENDED
P132_data_struct::~P132_data_struct() {
  delete INA;
  INA = nullptr;
}

# endif // if P132_EXTENDED

// **************************************************************************/
// Gets the raw bus voltage  (7FF8 / 32760) LSB 8mV
// **************************************************************************/
# if !P132_EXTENDED

int16_t P132_data_struct::getBusVoltage_raw(byte reg) {
  uint16_t value = I2C_read16_reg(_i2c_address, reg);

  // Shift to the right 3 to drop CNVR and OVF and multiply by LSB 8 mV
  #  ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLog(LOG_LEVEL_DEBUG,
           strformat(F("INA3221: get raw bus %d reg - %d"),
                     value, reg));
  }
  #  endif // ifndef BUILD_NO_DEBUG
  return (int16_t)((value >> 3) * 8);
}

// **************************************************************************/
// Gets the raw shunt voltage (integer, so +-32760) LSB 40 uV
// **************************************************************************/
int16_t P132_data_struct::getShuntVoltage_raw(byte reg) {
  uint16_t value = I2C_read16_reg(_i2c_address, reg);

  #  ifndef BUILD_NO_DEBUG
  String log = strformat(F("INA3221: get raw shunt voltage %d value2 - "), value);
  #  endif // ifndef BUILD_NO_DEBUG

  // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
  if (value > 32767) {               // check value is negative
    //		value = 0;  // no negative measure
    value = ((value >> 3) | 0xE000); // correct int16_t value
    #  ifndef BUILD_NO_DEBUG
    log += concat(F(" value_neg - "), value);
    #  endif // ifndef BUILD_NO_DEBUG
  } else {
    value = (value >> 3);
    #  ifndef BUILD_NO_DEBUG
    log += concat(F(" value_pos - "), value);
    #  endif // ifndef BUILD_NO_DEBUG
  }
  #  ifndef BUILD_NO_DEBUG
  log += concat(F(" reg - "), reg);
  addLog(LOG_LEVEL_DEBUG, log);
  #  endif // ifndef BUILD_NO_DEBUG
  return value;
}

# endif // if !P132_EXTENDED

// **************************************************************************/
// Gets the shunt voltage in mV
// **************************************************************************/
# if P132_EXTENDED
float P132_data_struct::getShuntVoltage_mV(uint8_t reg) {
  if (!isInitialized()) {
    return 0.0f;
  }
  return INA->getShuntMicroVolts(_device + reg) * 0.001f;
}

# else // if P132_EXTENDED
float P132_data_struct::getShuntVoltage_mV(byte reg) {
  int16_t value = getShuntVoltage_raw(reg);

  #  ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLog(LOG_LEVEL_DEBUG,
           strformat(F("INA3221: shunt voltage in mV * 0.04 %d reg - %d"),
                     value, reg));
  }
  #  endif // ifndef BUILD_NO_DEBUG
  return value * 0.04f;
}

# endif // if P132_EXTENDED

// **************************************************************************/
// Gets the Bus voltage in volts
// **************************************************************************/
# if P132_EXTENDED
float P132_data_struct::getBusVoltage_V(uint8_t reg) {
  if (!isInitialized()) {
    return 0.0f;
  }
  return INA->getBusMilliVolts(_device + reg) * 0.001f;
}

# else // if P132_EXTENDED
float P132_data_struct::getBusVoltage_V(byte reg) {
  int16_t value = getBusVoltage_raw(reg);

  #  ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLog(LOG_LEVEL_DEBUG,
           strformat(F("INA3221: get bus voltage %d reg - %d"),
                     value, reg));
  }
  #  endif // ifndef BUILD_NO_DEBUG
  return value * 0.001f;
}

# endif // if P132_EXTENDED

# if P132_EXTENDED

// **************************************************************************/
// Gets the Bus current in milliampere
// **************************************************************************/
float P132_data_struct::getBusCurrent_mA(uint8_t reg) {
  if (!isInitialized()) {
    return 0.0f;
  }
  return INA->getBusMicroAmps(_device + reg) * 0.001f;
}

// **************************************************************************/
// Gets the Bus power in milliwatt
// **************************************************************************/
float P132_data_struct::getBusPower_mW(uint8_t reg) {
  if (!isInitialized()) {
    return 0.0f;
  }
  return INA->getBusMicroWatts(_device + reg) * 0.001f;
}

bool P132_data_struct::conversionFinished(uint8_t reg) {
  if (!isInitialized()) {
    return false;
  }
  return INA->conversionFinished(_device + reg);
}

# endif // if P132_EXTENDED

// **************************************************************************/
// Configures the INA
// **************************************************************************/
# if P132_EXTENDED
void P132_data_struct::setCalibration(struct EventStruct *event) {
  if (!isInitialized()) {
    return;
  }

  if (P132_DeviceType::Ina219 != _deviceType) { // Averaging set via conversion
    INA->setAveraging(getAverageBitsToFactor(P132_GET_AVERAGE, _deviceType), _device);
  }
  const uint8_t convB = 0 == P132_GET_CFG_VERSION ? P132_GET_CONVERSION_S : P132_GET_V2_CONVERSION_S;
  const uint8_t convS = 0 == P132_GET_CFG_VERSION ? P132_GET_CONVERSION_S : P132_GET_V2_CONVERSION_S;
  INA->setBusConversion(getConversionBitsToFactor(convB, _deviceType), _device);
  INA->setShuntConversion(getConversionBitsToFactor(convS, _deviceType), _device);

  #  ifndef BUILD_NO_DEBUG

  // Config register and manufacurer id
  const uint32_t config = I2C_read16_reg(_i2c_address, 0x00); // read config
  const uint16_t mfgid  = I2C_read16_reg(_i2c_address, 0xFE); // read manufacturer ID
  const uint16_t dieid  = I2C_read16_reg(_i2c_address, 0xFF); // read Die ID


  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO,
           strformat(F("INA  : init I2C: 0x%02x mfg: 0x%x, did: 0x%x config: 0x%x, 0b%s"),
                     _i2c_address, mfgid, dieid, config, String(config, BIN).c_str()));
  }
  #  endif // ifndef BUILD_NO_DEBUG
}

# else // if P132_EXTENDED
void P132_data_struct::setCalibration_INA3221(struct EventStruct *event) {
  // Set Config register
  uint32_t config = I2C_read16_reg(_i2c_address, 0x00); // read, chip default: 0x7127
  uint16_t mfgid  = I2C_read16_reg(_i2c_address, 0xFE); // read manufacturer ID, should be 0x5449

  set3BitToUL(config, INA3221_AVERAGE_BIT,          P132_GET_AVERAGE);
  set3BitToUL(config, INA3221_CONVERSION_BUS_BIT,   P132_GET_CONVERSION_B);
  set3BitToUL(config, INA3221_CONVERSION_SHUNT_BIT, P132_GET_CONVERSION_S);

  #  ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO,
           strformat(F("INA3221: init I2C: 0x%02x mfg: 0x%x, config: 0x%x, 0b%s"),
                     _i2c_address, mfgid, config, String(config, BIN).c_str()));
  }
  #  endif // ifndef BUILD_NO_DEBUG

  if (mfgid != 0x5449) {
    addLogMove(LOG_LEVEL_ERROR, F("INA3221: Invalid Manufacturer ID! (0x5449)"));
  }

  I2C_write16_reg(_i2c_address, 0x00, static_cast<uint16_t>(config));
}

# endif // if P132_EXTENDED

# if P132_EXTENDED
uint32_t P132_data_struct::getAverageBitsToFactor(uint8_t bits, P132_DeviceType deviceType) {
  if (P132_DeviceType::Ina219 == deviceType) {
    if (bits) {
      return 1 << bits;
    }
    return 0u;
  }

  switch (bits) {
    case 0b000: return 0u;
    case 0b001: return 4u;
    case 0b010: return 16u;
    case 0b011: return 64u;
    case 0b100: return 128u;
    case 0b101: return 256u;
    case 0b110: return 512u;
    case 0b111: return 1024u;
  }
  return 0u;
}

uint32_t P132_data_struct::getConversionBitsToFactor(uint8_t bits, P132_DeviceType deviceType) {
  if (P132_DeviceType::Ina219 == deviceType) {
    switch (bits) {
      case 0b0000: return 0u;
      case 0b0001: return 148u;
      case 0b0010: return 276u;
      case 0b1000: return 532u;
      case 0b1001: return 1060u;
      case 0b1010: return 2130u;
      case 0b1011: return 4260u;
      case 0b1100: return 8510u;
      case 0b1101: return 17020u;
      case 0b1110: return 34050u;
      case 0b1111: return 68100u;
    }
    return 532u;
  }

  switch (bits) {
    case 0b0000: return 0u;
    case 0b0001: return 204u;
    case 0b0010: return 332u;
    case 0b0011: return 588u;
    case 0b0100: return 1100u;
    case 0b0101: return 2116u;
    case 0b0110: return 4156u;
    case 0b0111: return 8244u;
  }
  return 1100u;
}

# endif // if P132_EXTENDED

#endif // ifdef USES_P132
