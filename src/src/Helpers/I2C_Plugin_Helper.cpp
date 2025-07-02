#include "../Helpers/I2C_Plugin_Helper.h"

/***********************************************************************
 * checkI2CConfigValid_toHtml(taskIndex, onlyCheck)
 * Check if I2C is correctly configured and usable for this task
 * taskIndex: will be used in planned enhancements
 * outputToHtml = false: no html output is generated
 * Outputs an error message and returns false if not correct
 **********************************************************************/
bool checkI2CConfigValid_toHtml(taskIndex_t taskIndex,
                                bool        outputToHtml) {
  #if FEATURE_I2C_MULTIPLE
  const uint8_t i2cBus = Settings.getI2CInterface(taskIndex);
  #else
  const uint8_t i2cBus = 0;
  #endif // if FEATURE_I2C_MULTIPLE
  if ((Settings.getI2CSdaPin(i2cBus) == -1) || (Settings.getI2CSclPin(i2cBus) == -1)) {
    if (outputToHtml) { addHtml(F("Incomplete I2C configuration.")); }
    return false;
  }
  #if FEATURE_I2CMULTIPLEXER

  if ((Settings.getI2CMultiplexerType(i2cBus) != I2C_MULTIPLEXER_NONE) &&
      (Settings.getI2CMultiplexerAddr(i2cBus) == -1)) { // Multiplexer selected, but no port configured
    if (outputToHtml) { addHtml(F("Incomplete I2C Multiplexer configuration.")); }
    return false;
  }
  #endif // if FEATURE_I2CMULTIPLEXER
  return true;
}
