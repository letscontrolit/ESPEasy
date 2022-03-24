#include "../PluginStructs/P114_data_struct.h"

#ifdef USES_P114

// **************************************************************************/
// Constructor
// **************************************************************************/
P114_data_struct::P114_data_struct(uint8_t i2c_addr, uint8_t integration_time, bool highDensity)
  : i2cAddress(i2c_addr), IT(integration_time), HD(highDensity) {}


// **************************************************************************/
// Initialize sensor and read data from VEML6075
// **************************************************************************/
bool P114_data_struct::read_sensor(float& _UVA, float& _UVB, float& _UVIndex) {
  if (!initialised) {
    initialised = init_sensor(); // Check id device is present
  }


  #ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    log.reserve(40);
    log  = F("VEML6075: i2caddress: 0x");
    log += String(i2cAddress, HEX);
    addLogMove(LOG_LEVEL_DEBUG, log);
    log  = F("VEML6075: initialized: ");
    log += String(initialised ? F("true") : F("false"));
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
  #endif

  if (initialised) {
    for (int j = 0; j < 5; j++) {
      UVData[j] = I2C_read16_LE_reg(i2cAddress, VEML6075_UVA_DATA + j);
    }

    // Calculate the UV Index, valid in open air not behind glass!
    UVAComp  = (UVData[0] - UVData[1]) - ACoef * (UVData[3] - UVData[1]) - BCoef * (UVData[4] - UVData[1]);
    UVBComp  = (UVData[2] - UVData[1]) - CCoef * (UVData[3] - UVData[1]) - DCoef * (UVData[4] - UVData[1]);
    _UVIndex = ((UVBComp * UVBresponsivity) +  (UVAComp * UVAresponsivity)) / 2.0f;

    _UVA = static_cast<float>(UVData[0]) / static_cast<float>(1 << (IT - 1)); // UVA light sensitivity increases linear with integration time
    _UVB = static_cast<float>(UVData[2]) / static_cast<float>(1 << (IT - 1)); // UVB light sensitivity increases linear with integration time

    // float UVASensitivity = 0.93/(static_cast<float>(IT + 1)); // UVA light sensitivity increases with integration time
    // float UVBSensitivity = 2.10/(static_cast<float>(IT + 1)); // UVB light sensitivity increases with integration time
    #ifndef BUILD_NO_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log  = F("VEML6075: IT raw: 0x");
      log += String(IT + 1, HEX);
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
    #endif
    return true;
  }
  return false;
}

// **************************************************************************/
// Check VEML6075 presence and initialize
// **************************************************************************/
bool P114_data_struct::init_sensor() {
  uint16_t deviceID = I2C_readS16_LE_reg(i2cAddress, VEML6075_UV_ID);

  #ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    log.reserve(60);
    log  = F("VEML6075: ID: 0x");
    log += String(deviceID, HEX);
    log += F(" / checked Address: 0x");
    log += String(i2cAddress, HEX);
    log += F(" / 0x");
    log += String(VEML6075_UV_ID, HEX);
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
  #endif

  if (deviceID != 0x26) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log;
      log.reserve(60);
      log  = F("VEML6075: wrong deviceID: ");
      log += String(deviceID, HEX);
      addLogMove(LOG_LEVEL_ERROR, log);
    }
    return false;
  } else {

    // log  = F("VEML6075: found deviceID: 0x");
    // log += String(deviceID, HEX);

    if (!I2C_write16_LE_reg(i2cAddress, VEML6075_UV_CONF, (IT << 4) | (HD << 3))) { // Bit 3 must be 0, bit 0 is 0 for run and 1 for
      // shutdown, LS Byte
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String log;
        log.reserve(60);
        log  = F("VEML6075: setup failed!!");
        log += F(" / CONF: ");
        log += String(static_cast<uint16_t>(IT << 4) | (HD << 3), BIN);
        addLogMove(LOG_LEVEL_ERROR, log);
      }
      return false;
    } else if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log  = F("VEML6075: sensor initialised / CONF: ");
      log += String((uint16_t)(IT << 4) | (HD << 3), BIN);
      addLogMove(LOG_LEVEL_INFO, log);
    }
  }
  return true;
}

#endif // ifdef USES_P114
