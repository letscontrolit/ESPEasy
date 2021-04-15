#include "../PluginStructs/P015_data_struct.h"
#ifdef USES_P015

#include "../Helpers/Misc.h"



# define TSL2561_CMD           0x80
# define TSL2561_REG_CONTROL   0x00
# define TSL2561_REG_TIMING    0x01
# define TSL2561_REG_DATA_0    0x0C
# define TSL2561_REG_DATA_1    0x0E


P015_data_struct::P015_data_struct(byte i2caddr, unsigned int gain, byte integration) :
  _gain(gain),
  _i2cAddr(i2caddr),
  _integration(integration)
{
  // If gain = false (0), device is set to low gain (1X)
  // If gain = high (1), device is set to high gain (16X)

  _gain16xActive = gain == 1;

  if (!useAutoGain()) {
    _gain16xActive = gain == 1;
  }
}

bool P015_data_struct::performRead(float& luxVal,
                                   float& infraredVal,
                                   float& broadbandVal,
                                   float& ir_broadband_ratio)
{
  bool success = false;
  int  attempt = useAutoGain() ? 2 : 1;

  while (!success && attempt > 0) {
    --attempt;

    float ms; // Integration ("shutter") time in milliseconds

    // If time = 0, integration will be 13.7ms
    // If time = 1, integration will be 101ms
    // If time = 2, integration will be 402ms
    unsigned char time = _integration;

    plugin_015_setTiming(_gain16xActive, time, ms);
    setPowerUp();
    delayBackground(ms); // FIXME TD-er: Do not use delayBackground but collect data later.
    unsigned int data0, data1;

    if (getData(data0, data1))
    {
      float lux;       // Resulting lux value
      float infrared;  // Resulting infrared value
      float broadband; // Resulting broadband value


      // Perform lux calculation:
      success = !ADC_saturated(time,  data0) && !ADC_saturated(time, data1);
      getLux(_gain16xActive, ms, data0, data1, lux, infrared, broadband);

      if (useAutoGain()) {
        if (_gain16xActive) {
          // Last reading was using 16x gain
          // Check using some margin to see if gain is still needed
          if (ADC_saturated(time,  data0 * 16)) {
            _gain16xActive = false;
          }
        } else {
          // Check using some margin to see if gain will improve reading resolution
          if (lux < 40) {
            _gain16xActive = true;
          }
        }
      }

      if (success) {
        if (broadband > 0.0f) {
          // Store the ratio in an unused user var. (should we make it available?)
          // Only store/update it when not close to the limits of both ADC ranges.
          // When using this value to compute extended ranges, it must not be using a ratio taken from a
          // heated sensor, since then the IR part may be off quite a bit resulting in very unrealistic values.
          if (!ADC_saturated(time,  data0 * 2) && !ADC_saturated(time, data1 * 2)) {
            ir_broadband_ratio = infrared / broadband;
          }
        }
      } else {
        // Use last known ratio to reconstruct the broadband value
        // If IR is saturated, output the max value based on the last known ratio.
        if ((ir_broadband_ratio > 0.0f) && (_gain == P015_EXT_AUTO_GAIN)) {
          data0 = static_cast<float>(data1) / ir_broadband_ratio;
          getLux(_gain16xActive, ms, data0, data1, lux, infrared, broadband);
          success = true;
        }
      }
      luxVal       = lux;
      infraredVal  = infrared;
      broadbandVal = broadband;
    }
    else
    {
      // getData() returned false because of an I2C error, inform the user.
      addLog(LOG_LEVEL_ERROR, F("TSL2561: i2c error"));
      success = false;
      attempt = 0;
    }
  }
  return success;
}

bool P015_data_struct::useAutoGain() const
{
  const bool autoGain = _gain == P015_AUTO_GAIN || _gain == P015_EXT_AUTO_GAIN;

  return autoGain;
}

bool P015_data_struct::begin()
{
  // Wire.begin();   called in ESPEasy framework
  return true;
}

bool P015_data_struct::readByte(unsigned char address, unsigned char& value)

// Reads a byte from a TSL2561 address
// Address: TSL2561 address (0 to 15)
// Value will be set to stored byte
// Returns true (1) if successful, false (0) if there was an I2C error
{
  // Set up command byte for read
  Wire.beginTransmission(_i2cAddr);
  Wire.write((address & 0x0F) | TSL2561_CMD);
  _error = Wire.endTransmission();

  // Read requested byte
  if (_error == 0)
  {
    Wire.requestFrom(_i2cAddr, (byte)1);

    if (Wire.available() == 1)
    {
      value = Wire.read();
      return true;
    }
  }
  return false;
}

bool P015_data_struct::writeByte(unsigned char address, unsigned char value)

// Write a byte to a TSL2561 address
// Address: TSL2561 address (0 to 15)
// Value: byte to write to address
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() above)
{
  // Set up command byte for write
  Wire.beginTransmission(_i2cAddr);
  Wire.write((address & 0x0F) | TSL2561_CMD);

  // Write byte
  Wire.write(value);
  _error = Wire.endTransmission();

  if (_error == 0) {
    return true;
  }

  return false;
}

bool P015_data_struct::readUInt(unsigned char address, unsigned int& value)

// Reads an unsigned integer (16 bits) from a TSL2561 address (low byte first)
// Address: TSL2561 address (0 to 15), low byte first
// Value will be set to stored unsigned integer
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() above)
{
  // Set up command byte for read
  Wire.beginTransmission(_i2cAddr);
  Wire.write((address & 0x0F) | TSL2561_CMD);
  _error = Wire.endTransmission();

  // Read two bytes (low and high)
  if (_error == 0)
  {
    Wire.requestFrom(_i2cAddr, (byte)2);

    if (Wire.available() == 2)
    {
      char high, low;
      low  = Wire.read();
      high = Wire.read();

      // Combine bytes into unsigned int
      value = word(high, low);
      return true;
    }
  }
  return false;
}

bool P015_data_struct::writeUInt(unsigned char address, unsigned int value)

// Write an unsigned integer (16 bits) to a TSL2561 address (low byte first)
// Address: TSL2561 address (0 to 15), low byte first
// Value: unsigned int to write to address
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() above)
{
  // Split int into lower and upper bytes, write each byte
  if (writeByte(address, lowByte(value))
      && writeByte(address + 1, highByte(value))) {
    return true;
  }

  return false;
}

bool P015_data_struct::plugin_015_setTiming(bool gain, unsigned char time)

// If gain = false (0), device is set to low gain (1X)
// If gain = high (1), device is set to high gain (16X)
// If time = 0, integration will be 13.7ms
// If time = 1, integration will be 101ms
// If time = 2, integration will be 402ms
// If time = 3, use manual start / stop
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() below)
{
  unsigned char timing;

  // Get timing byte
  if (readByte(TSL2561_REG_TIMING, timing))
  {
    // Set gain (0 or 1)
    if (gain) {
      timing |= 0x10;
    }
    else {
      timing &= ~0x10;
    }

    // Set integration time (0 to 3)
    timing &= ~0x03;
    timing |= (time & 0x03);

    // Write modified timing byte back to device
    if (writeByte(TSL2561_REG_TIMING, timing)) {
      return true;
    }
  }
  return false;
}

bool P015_data_struct::plugin_015_setTiming(bool gain, unsigned char time, float& ms)

// If gain = false (0), device is set to low gain (1X)
// If gain = high (1), device is set to high gain (16X)
// If time = 0, integration will be 13.7ms
// If time = 1, integration will be 101ms
// If time = 2, integration will be 402ms
// If time = 3, use manual start / stop (ms = 0)
// ms will be set to integration time
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() below)
{
  // Calculate ms for user
  switch (time)
  {
    case 0: ms  = 13.7f; break;
    case 1: ms  = 101; break;
    case 2: ms  = 402; break;
    default: ms = 402; // used in a division, so do not use 0
  }

  // Set integration using base function
  return plugin_015_setTiming(gain, time);
}

// Determine if either sensor saturated (max depends on clock freq. and integration time)
// If so, abandon ship (calculation will not be accurate)
bool P015_data_struct::ADC_saturated(unsigned char time, unsigned int value) {
  unsigned int max_ADC_count = 65535;

  switch (time)
  {
    case 0: max_ADC_count = 5047; break;
    case 1: max_ADC_count = 37177; break;
    case 2:
    default: break;
  }
  return value >= max_ADC_count;
}

bool P015_data_struct::setPowerUp(void)

// Turn on TSL2561, begin integrations
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() below)
{
  // Write 0x03 to command byte (power on)
  return writeByte(TSL2561_REG_CONTROL, 0x03);
}

bool P015_data_struct::setPowerDown(void)

// Turn off TSL2561
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() below)
{
  // Clear command byte (power off)
  return writeByte(TSL2561_REG_CONTROL, 0x00);
}

bool P015_data_struct::getData(unsigned int& data0, unsigned int& data1)

// Retrieve raw integration results
// data0 and data1 will be set to integration results
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() below)
{
  // Get data0 and data1 out of result registers
  if (readUInt(TSL2561_REG_DATA_0, data0) && readUInt(TSL2561_REG_DATA_1, data1)) {
    return true;
  }

  return false;
}

void P015_data_struct::getLux(unsigned char gain,
                              float         ms,
                              unsigned int  CH0,
                              unsigned int  CH1,
                              float       & lux,
                              float       & infrared,
                              float       & broadband)

// Convert raw data to lux
// gain: 0 (1X) or 1 (16X), see setTiming()
// ms: integration time in ms, from setTiming() or from manual integration
// CH0, CH1: results from getData()
// lux will be set to resulting lux calculation
// returns true (1) if calculation was successful
// RETURNS false (0) AND lux = 0.0 IF EITHER SENSOR WAS SATURATED (0XFFFF)
{
  float ratio, d0, d1;

  // Convert from unsigned integer to floating point
  d0 = CH0; d1 = CH1;

  // We will need the ratio for subsequent calculations
  ratio = d1 / d0;

  // save original values
  infrared  = d1;
  broadband = d0;

  // Normalize for integration time
  d0 *= (402.0f / ms);
  d1 *= (402.0f / ms);

  // Normalize for gain
  if (!gain)
  {
    d0 *= 16;
    d1 *= 16;
  }

  // Determine lux per datasheet equations:
  if (ratio < 0.5f)
  {
    lux = 0.0304f * d0 - 0.062f * d0 * pow(ratio, 1.4);
  } else if (ratio < 0.61f)
  {
    lux = 0.0224f * d0 - 0.031f * d1;
  } else if (ratio < 0.80f)
  {
    lux = 0.0128f * d0 - 0.0153f * d1;
  } else if (ratio < 1.30f)
  {
    lux = 0.00146f * d0 - 0.00112f * d1;
  } else {
    // ratio >= 1.30
    lux = 0.0f;
  }
}

#endif // ifdef USES_P015
