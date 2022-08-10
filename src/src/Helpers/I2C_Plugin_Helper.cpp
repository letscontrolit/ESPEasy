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
  if ((Settings.Pin_i2c_sda == -1) || (Settings.Pin_i2c_scl == -1)) {
    if (outputToHtml) { addHtml(F("Incomplete I2C configuration.")); }
    return false;
  }
  #if FEATURE_I2CMULTIPLEXER

  if ((Settings.I2C_Multiplexer_Type != I2C_MULTIPLEXER_NONE) &&
      (Settings.I2C_Multiplexer_Addr == -1)) { // Multiplexer selected, but no port configured
    if (outputToHtml) { addHtml(F("Incomplete I2C Multiplexer configuration.")); }
    return false;
  }
  #endif // if FEATURE_I2CMULTIPLEXER
  return true;
}
