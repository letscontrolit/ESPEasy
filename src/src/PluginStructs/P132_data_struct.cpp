#include "../PluginStructs/P132_data_struct.h"

#ifdef USES_P132

// **************************************************************************/
// Constructor
// **************************************************************************/
P132_data_struct::P132_data_struct(struct EventStruct *event) {
  _i2c_address = P132_I2C_ADDR;
  setCalibration_INA3221(event);
}

// **************************************************************************/
// Gets the raw bus voltage  (7FF8 / 32760) LSB 8mV
// **************************************************************************/
int16_t P132_data_struct::getBusVoltage_raw(byte reg) {
  uint16_t value = I2C_read16_reg(_i2c_address, reg);

  // Shift to the right 3 to drop CNVR and OVF and multiply by LSB 8 mV
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLog(LOG_LEVEL_DEBUG,
           strformat(F("INA3221: get raw bus %d reg - %d"),
                     value, reg));
  }
  # endif // ifndef BUILD_NO_DEBUG
  return (int16_t)((value >> 3) * 8);
}

// **************************************************************************/
// Gets the raw shunt voltage (integer, so +-32760) LSB 40 uV
// **************************************************************************/
int16_t P132_data_struct::getShuntVoltage_raw(byte reg) {
  uint16_t value = I2C_read16_reg(_i2c_address, reg);

  # ifndef BUILD_NO_DEBUG
  String log = strformat(F("INA3221: get raw shunt voltage %d value2 - "), value);
  # endif // ifndef BUILD_NO_DEBUG

  // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
  if (value > 32767) {               // check value is negative
    //		value = 0;  // no negative measure
    value = ((value >> 3) | 0xE000); // correct int16_t value
    # ifndef BUILD_NO_DEBUG
    log += concat(F(" value_neg - "), value);
    # endif // ifndef BUILD_NO_DEBUG
  } else {
    value = (value >> 3);
    # ifndef BUILD_NO_DEBUG
    log += concat(F(" value_pos - "), value);
    # endif // ifndef BUILD_NO_DEBUG
  }
  # ifndef BUILD_NO_DEBUG
  log += concat(F(" reg - "), reg);
  addLog(LOG_LEVEL_DEBUG, log);
  # endif // ifndef BUILD_NO_DEBUG
  return value;
}

// **************************************************************************/
// Gets the shunt voltage in mV (32760 so +-163.8 mV) 7ff8 LSB 40uV
// **************************************************************************/
float P132_data_struct::getShuntVoltage_mV(byte reg) {
  int16_t value = getShuntVoltage_raw(reg);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLog(LOG_LEVEL_DEBUG,
           strformat(F("INA3221: shunt voltage in mV * 0.04 %d reg - %d"),
                     value, reg));
  }
  # endif // ifndef BUILD_NO_DEBUG
  return value * 0.04f;
}

// **************************************************************************/
// Gets the Bus voltage in volts
// **************************************************************************/
float P132_data_struct::getBusVoltage_V(byte reg) {
  int16_t value = getBusVoltage_raw(reg);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLog(LOG_LEVEL_DEBUG,
           strformat(F("INA3221: get bus voltage %d reg - %d"),
                     value, reg));
  }
  # endif // ifndef BUILD_NO_DEBUG
  return value * 0.001f;
}

// **************************************************************************/
// Configures to INA3221
// **************************************************************************/
void P132_data_struct::setCalibration_INA3221(struct EventStruct *event) {
  // Set Config register
  uint32_t config = I2C_read16_reg(_i2c_address, 0x00); // read, chip default: 0x7127
  uint16_t mfgid  = I2C_read16_reg(_i2c_address, 0xFE); // read manufacturer ID, should be 0x5449

  set3BitToUL(config, INA3221_AVERAGE_BIT,          P132_GET_AVERAGE);
  set3BitToUL(config, INA3221_CONVERSION_BUS_BIT,   P132_GET_CONVERSION_B);
  set3BitToUL(config, INA3221_CONVERSION_SHUNT_BIT, P132_GET_CONVERSION_S);

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO,
           strformat(F("INA3221: init I2C: 0x%02x mfg: 0x%x, config: 0x%x, 0b%s"),
                     _i2c_address, mfgid, config, String(config, BIN).c_str()));
  }
  # endif // ifndef BUILD_NO_DEBUG

  if (mfgid != 0x5449) {
    addLogMove(LOG_LEVEL_ERROR, F("INA3221: Invalid Manufacturer ID! (0x5449)"));
  }

  I2C_write16_reg(_i2c_address, 0x00, static_cast<uint16_t>(config));
}

#endif // ifdef USES_P132
