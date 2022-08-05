#include "../PluginStructs/P014_data_struct.h"

#ifdef USES_P014
P014_data_struct::P014_data_struct(uint8_t resolution) : res(resolution) {
  reset();
}

void P014_data_struct::reset()
{
  state         = SI7021_state::Uninitialized; // Force device setup next time
  timeStartRead = 0;
}

bool P014_data_struct::init()
{
  state = SI7021_state::Uninitialized;

  // Set the resolution we want
  const uint8_t ret = setResolution(res);

  if (ret == 0) {
    state = SI7021_state::Initialized;
    return true;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("SI7021 : Res=0x");
    log += String(res, HEX);
    log += F(" => Error 0x");
    log += String(ret, HEX);
    addLogMove(LOG_LEVEL_INFO, log);
  }
  return false;
}

bool P014_data_struct::loop() {
  switch (state)
  {
    case SI7021_state::Initialized:
    {
      // Start conversion for humidity
      startConv(SI7021_MEASURE_HUM);
      timeStartRead = millis();

      // change state of sensor
      state = SI7021_state::Wait_for_temperature_samples;
      break;
    }

    case SI7021_state::Wait_for_temperature_samples:
    {
      // Check if conversion is finished
      if (readValues(SI7021_MEASURE_HUM, res) == 0) {
        // Start conversion for temperature
        startConv(SI7021_MEASURE_TEMP);

        // change state of sensor
        state = SI7021_state::Wait_for_humidity_samples;
      }
      break;
    }

    case SI7021_state::Wait_for_humidity_samples:
    {
      // Check if conversion is finished
      if (readValues(SI7021_MEASURE_TEMP, res) == 0) {
        // change state of sensor
        state         = SI7021_state::New_values;
        timeStartRead = 0;
      }
      break;
    }

    default:
      break;
  }

  if (timeStartRead != 0) {
    // Apparently we're waiting for some reading.
    if (timePassedSince(timeStartRead) > SI7021_TIMEOUT) {
      reset();
    }
  }
  return SI7021_state::New_values == state;
}

bool P014_data_struct::getReadValue(float& temperature, float& humidity) {
  bool success = false;

  switch (state) {
    case SI7021_state::Uninitialized:
    {
      addLog(LOG_LEVEL_INFO, F("SI7021 : sensor not initialized !"));
      init();
      break;
    }
    case SI7021_state::Initialized:
    {
      // No call made to start a new reading
      // Should be handled in the loop()
      addLog(LOG_LEVEL_INFO, F("SI7021 : No read started !"));
      break;
    }
    case SI7021_state::Wait_for_temperature_samples:
    case SI7021_state::Wait_for_humidity_samples:
    {
      // Still waiting for data
      // Should be handled in the loop()
      addLog(LOG_LEVEL_ERROR, F("SI7021 : Read Error !"));
      break;
    }

    case SI7021_state::New_values:
    {
      temperature = si7021_temperature / 100.0f;
      humidity    = si7021_humidity / 10.0f;
      state       = SI7021_state::Values_read;
      success     = true;

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("SI7021 : Temperature: ");
        log += temperature;
        addLogMove(LOG_LEVEL_INFO, log);
        log  = F("SI7021 : Humidity: ");
        log += humidity;
        addLogMove(LOG_LEVEL_INFO, log);
      }
      break;
    }
    case SI7021_state::Values_read:
    {
      // This must be done in a separate call to
      // make sure we only start reading when the plugin wants us to perform a reading.
      // Change state of sensor for non blocking reading
      state = SI7021_state::Initialized;
      break;
    }
  }
  return success;
}

/* ======================================================================
   Function: checkCRC
   Purpose : check the CRC of received data
   Input   : value read from sensor
   Output  : CRC read from sensor
   Comments: 0 if okay
   ====================================================================== */
uint8_t P014_data_struct::checkCRC(uint16_t data, uint8_t check)
{
  uint32_t remainder, divisor;

  // Pad with 8 bits because we have to add in the check value
  remainder = (uint32_t)data << 8;

  // From: http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
  // POLYNOMIAL = 0x0131 = x^8 + x^5 + x^4 + 1 : http://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks
  // 0x988000 is the 0x0131 polynomial shifted to farthest left of three bytes
  divisor = (uint32_t)0x988000;

  // Add the check value
  remainder |= check;

  // Operate on only 16 positions of max 24.
  // The remaining 8 are our remainder and should be zero when we're done.
  for (uint8_t i = 0; i < 16; i++) {
    // Check if there is a one in the left position
    if (remainder & (uint32_t)1 << (23 - i)) {
      remainder ^= divisor;
    }

    // Rotate the divisor max 16 times so that we have 8 bits left of a remainder
    divisor >>= 1;
  }
  return (uint8_t)remainder;
}

/* ======================================================================
   Function: si7021_readRegister
   Purpose : read the user register from the sensor
   Input   : user register value filled by function
   Output  : 0 if okay
   Comments: -
   ====================================================================== */
int8_t P014_data_struct::readRegister(uint8_t *value)
{
  // Request user register
  Wire.beginTransmission(SI7021_I2C_ADDRESS);
  Wire.write(SI7021_READ_REG);
  Wire.endTransmission();

  // request 1 uint8_t result
  Wire.requestFrom(SI7021_I2C_ADDRESS, 1);

  if (Wire.available() >= 1) {
    *value = Wire.read();
    return 0;
  }

  return 1;
}

/* ======================================================================
   Function: startConv
   Purpose : return temperature or humidity measured
   Input   : data type SI7021_READ_HUM or SI7021_READ_TEMP
   Output  : 0 if okay
   Comments: -
   ====================================================================== */
uint8_t P014_data_struct::startConv(uint8_t datatype)
{
  // Request a reading
  Wire.beginTransmission(SI7021_I2C_ADDRESS);
  Wire.write(datatype);
  Wire.endTransmission();

  return 0;
}

/* ======================================================================
   Function: readValues
   Purpose : read temperature and humidity from SI7021 sensor
   Input   : current config resolution
   Output  : 0 if okay
   Comments: -
   ====================================================================== */
int8_t P014_data_struct::readValues(uint8_t datatype, uint8_t resolution)
{
  long data;
  uint16_t raw;
  uint8_t  checksum;

  if (Wire.requestFrom(SI7021_I2C_ADDRESS, 3) != 3) {
    return -1;
  }

  // Comes back in three bytes, data(MSB) / data(LSB) / Checksum
  raw      = ((uint16_t)Wire.read()) << 8;
  raw     |= Wire.read();
  checksum = Wire.read();

  // Check CRC of data received
  if (checkCRC(raw, checksum) != 0) {
    addLog(LOG_LEVEL_ERROR, F("SI7021 : checksum error!"));
    return -1;
  }

  // Humidity
  if ((datatype == SI7021_MEASURE_HUM) || (datatype == SI7021_MEASURE_HUM_HM)) {
    // Convert value to Himidity percent
    // pm-cz: it is possible to enable decimal places for humidity as well by multiplying the value in formula by 100
    data = ((1250 * (long)raw) >> 16) - 60;

    // Datasheet says doing this check
    if (data > 1000) { data = 1000; }

    if (data < 0) { data = 0; }

    // pm-cz: Let us make sure we have enough precision due to ADC bits
    if (resolution == SI7021_RESOLUTION_12T_08RH) {
      data  = (data + 5) / 10;
      data *= 10;
    }

    // save value
    si7021_humidity = (uint16_t)data;

    // Temperature
  } else if ((datatype == SI7021_MEASURE_TEMP) || (datatype == SI7021_MEASURE_TEMP_HM) || (datatype == SI7021_MEASURE_TEMP_HUM)) {
    // Convert value to Temperature (*100)
    // for 23.45C value will be 2345
    data =  ((17572 * (long)raw) >> 16) - 4685;

    /*
       // pm-cz: We should probably check for precision here as well
       if (resolution != SI7021_RESOLUTION_14T_12RH) {
       if (data > 0) {
        data = (data + 5) / 10;
       } else {
        data = (data - 5) / 10;
       }
       data *= 10;
       }
     */

    // save value
    si7021_temperature = (int16_t)data;
  }

  return 0;
}

/* ======================================================================
   Function: setResolution
   Purpose : Sets the sensor resolution to one of four levels
   Input   : see #define default is SI7021_RESOLUTION_14T_12RH
   Output  : 0 if okay
   Comments: -
   ====================================================================== */
int8_t P014_data_struct::setResolution(uint8_t res)
{
  // Get the current register value
  uint8_t reg   = 0;
  uint8_t error = readRegister(&reg);

  if (error == 0) {
    // remove resolution bits
    reg &= SI7021_RESOLUTION_MASK;

    // Prepare to write to the register value
    Wire.beginTransmission(SI7021_I2C_ADDRESS);
    Wire.write(SI7021_WRITE_REG);

    // Write the new resolution bits but clear unused before
    Wire.write(reg | (res &= ~SI7021_RESOLUTION_MASK));
    return (int8_t)Wire.endTransmission();
  }

  return error;
}

#endif // ifdef USES_P014
