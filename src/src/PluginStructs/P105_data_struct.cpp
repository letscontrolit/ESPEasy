#ifdef USES_P105

#include "../PluginStructs/P105_data_struct.h"


uint8_t AHTSetCalCmd[3]   = { AHTX_CMD, 0x08, 0x00 }; // load factory calibration coeff
uint8_t AHTSetCycleCmd[3] = { AHTX_CMD, 0x28, 0x00 }; // enable cycle mode
uint8_t AHTMeasureCmd[3]  = { 0xAC, 0x33, 0x00 };     // start measurement command
uint8_t AHTResetCmd       =   0xBA;                   // soft reset command


P105_data_struct::P105_data_struct(uint8_t addr) :
  last_hum_val(0.0f),
  last_temp_val(0.0f),
  last_measurement(0),
  i2cAddress(addr),
  state(AHT_state::Uninitialized) {}


bool P105_data_struct::initialized() const {
  return state != Uninitialized;
}

// Only perform the measurements with big interval to prevent the sensor from warming up.
bool P105_data_struct::update(unsigned long task_index) {
  const unsigned long current_time = millis();

  if (!initialized()) {
    if (!begin()) {
      return false;
    }
    state            = Initialized;
    last_measurement = 0;
  }

  if (state != Wait_for_samples) {
    last_measurement = current_time;

    startMeasurement();
    state = Wait_for_samples;
    return false;
  }

  // make sure we wait for the measurement to complete
  if (!timeOutReached(last_measurement + AHT10_MEASURMENT_DELAY)) {
    return false;
  }

  if (!readMeasurement()) {
    state = Initialized;
    return false;
  }

  last_measurement = current_time;
  state            = New_values;
  last_temp_val    = readTemperature();
  last_hum_val     = readHumidity();


  /*
     String log;

     if (loglevelActiveFor(LOG_LEVEL_INFO)) {
     log.reserve(120); // Prevent re-allocation
     log  = getDeviceName();
     log += F(":");
     }
     boolean logAdded = false;


     if ((tempOffset > 0.1f) || (tempOffset < -0.1f)) {
     // There is some offset to apply.
     if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" Apply temp offset ");
      log += tempOffset;
      log += F("C");
     }

     if (hasHumidity()) {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F(" humidity ");
        log += last_hum_val;
      }
      last_hum_val = compute_humidity_from_dewpoint(last_temp_val + tempOffset, last_dew_temp_val);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F("% => ");
        log += last_hum_val;
        log += F("%");
      }
     } else {
      last_hum_val = 0.0f;
     }

     if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" temperature ");
      log += last_temp_val;
     }
     last_temp_val = last_temp_val + tempOffset;

     if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log     += F("C => ");
      log     += last_temp_val;
      log     += F("C");
      logAdded = true;
     }
     }

     if (hasHumidity()) {
     if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log     += F(" dew point ");
      log     += last_dew_temp_val;
      log     += F("C");
      logAdded = true;
     }
     }

     if (logAdded && loglevelActiveFor(LOG_LEVEL_INFO)) {
     addLog(LOG_LEVEL_INFO, log);
     }

   */

  return true;
}

bool P105_data_struct::begin() {
  Wire.beginTransmission(i2cAddress);
  Wire.write(AHTResetCmd);
  Wire.endTransmission();

  delay(AHT10_SOFT_RESET_DELAY);

  Wire.beginTransmission(i2cAddress);
  Wire.write(AHTSetCalCmd, 3);

  if (Wire.endTransmission() != 0) {
    return false;
  }

  delay(AHT10_MEASURMENT_DELAY);

  if (readStatus(i2cAddress) & 0x08) { // Sensor calibrated?
    return true;
  }

  return false;
}

/**************************************************************************/

/*
    readStatusByte()
    Read status byte from sensor over I2C
 */

/**************************************************************************/
unsigned inline char P105_data_struct::readStatus(uint8_t address) {
  // uint8_t result = 0;
  // Need for AHT20?
  // Wire.beginTransmission(aht1x_address);
  // Wire.write(0x71);
  // if (Wire.endTransmission() != 0) return false;
  Wire.requestFrom(address, (uint8_t)1);
  return Wire.read();
}

/**************************************************************************/

/*
    readTemperature()
    Read temperature, °C
    NOTE:
    - temperature range      -40°C..+80°C
    - temperature resolution 0.01°C
    - temperature accuracy   ±0.3°C
 */

/**************************************************************************/
float P105_data_struct::readTemperature()
{
  if (AHT10_rawDataBuffer[0] == AHT10_ERROR) { return AHT10_ERROR;                                                                             //
                                                                                                                                               // error
                                                                                                                                               // handler,
                                                                                                                                               // collision
                                                                                                                                               // on
                                                                                                                                               // I2C
                                                                                                                                               // bus
  }
  uint32_t temperature = ((uint32_t)(AHT10_rawDataBuffer[3] & 0x0F) << 16) | ((uint32_t)AHT10_rawDataBuffer[4] << 8) | AHT10_rawDataBuffer[5]; //
                                                                                                                                               // 20-bit
                                                                                                                                               // raw
                                                                                                                                               // temperature
                                                                                                                                               // data

  return static_cast<float>(temperature) * 0.000191f - 50.0f;
}

/**************************************************************************/

/*
    readHumidity()
    Read relative humidity, %
    NOTE:
    - prolonged exposure for 60 hours at humidity > 80% can lead to a
      temporary drift of the signal +3%. Sensor slowly returns to the
      calibrated state at normal operating conditions.
    - relative humidity range      0%..100%
    - relative humidity resolution 0.024%
    - relative humidity accuracy   ±2%
 */

/**************************************************************************/
float P105_data_struct::readHumidity()
{
  if (AHT10_rawDataBuffer[0] == AHT10_ERROR) { return AHT10_ERROR;                                                                         //
                                                                                                                                           // error
                                                                                                                                           // handler,
                                                                                                                                           // collision
                                                                                                                                           // on
                                                                                                                                           // I2C
                                                                                                                                           // bus
  }
  uint32_t rawData = (((uint32_t)AHT10_rawDataBuffer[1] << 16) | ((uint32_t)AHT10_rawDataBuffer[2] << 8) | (AHT10_rawDataBuffer[3])) >> 4; //
                                                                                                                                           // 20-bit
                                                                                                                                           // raw
                                                                                                                                           // humidity
                                                                                                                                           // data

  const float humidity = static_cast<float>(rawData) * 0.000095f;

  if (humidity < 0) { return 0; }

  if (humidity > 100) { return 100; }

  return humidity;
}

bool P105_data_struct::startMeasurement() {
  Wire.beginTransmission(i2cAddress);
  Wire.write(AHTMeasureCmd, 3);

  if (Wire.endTransmission() != 0) {
    return false;
  }
  return true;
}

bool P105_data_struct::readMeasurement() {
  Wire.requestFrom(i2cAddress, (uint8_t)6);

  for (uint8_t i = 0; Wire.available() > 0 && i < 6; i++) {
    AHT10_rawDataBuffer[i] = Wire.read();
  }

  if (AHT10_rawDataBuffer[0] & 0x80) {
    return false; // device is busy
  }
  return true;
}

#endif // ifdef USES_P105
