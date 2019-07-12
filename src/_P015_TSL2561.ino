#ifdef USES_P015

// #######################################################################################################
// ######################## Plugin 015 TSL2561 I2C Lux Sensor ############################################
// #######################################################################################################
// complete rewrite, to support lower lux values better, add ability to change gain and sleep mode
// by: https://github.com/krikk
// this plugin is based on the sparkfun library
// written based on version 1.1.0 from https://github.com/sparkfun/SparkFun_TSL2561_Arduino_Library


#define PLUGIN_015
#define PLUGIN_ID_015        15
#define PLUGIN_NAME_015       "Light/Lux - TSL2561"
#define PLUGIN_VALUENAME1_015 "Lux"
#define PLUGIN_VALUENAME2_015 "Infrared"
#define PLUGIN_VALUENAME3_015 "Broadband"
#define PLUGIN_VALUENAME4_015 "Ratio"

bool Plugin_015_init = false;


#define TSL2561_ADDR_0 0x29 // address with '0' shorted on board
#define TSL2561_ADDR   0x39 // default address
#define TSL2561_ADDR_1 0x49 // address with '1' shorted on board

#define TSL2561_CMD           0x80
#define TSL2561_REG_CONTROL   0x00
#define TSL2561_REG_TIMING    0x01
#define TSL2561_REG_DATA_0    0x0C
#define TSL2561_REG_DATA_1    0x0E

#define P015_NO_GAIN          0
#define P015_16X_GAIN         1
#define P015_AUTO_GAIN        2
#define P015_EXT_AUTO_GAIN    3


byte plugin_015_i2caddr;
byte _error;

bool plugin_015_begin()
{
  // Wire.begin();   called in ESPEasy framework
  return true;
}

bool plugin_015_readByte(unsigned char address, unsigned char& value)

// Reads a byte from a TSL2561 address
// Address: TSL2561 address (0 to 15)
// Value will be set to stored byte
// Returns true (1) if successful, false (0) if there was an I2C error
{
  // Set up command byte for read
  Wire.beginTransmission(plugin_015_i2caddr);
  Wire.write((address & 0x0F) | TSL2561_CMD);
  _error = Wire.endTransmission();

  // Read requested byte
  if (_error == 0)
  {
    Wire.requestFrom(plugin_015_i2caddr, (byte)1);

    if (Wire.available() == 1)
    {
      value = Wire.read();
      return true;
    }
  }
  return false;
}

bool plugin_015_writeByte(unsigned char address, unsigned char value)

// Write a byte to a TSL2561 address
// Address: TSL2561 address (0 to 15)
// Value: byte to write to address
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() above)
{
  // Set up command byte for write
  Wire.beginTransmission(plugin_015_i2caddr);
  Wire.write((address & 0x0F) | TSL2561_CMD);

  // Write byte
  Wire.write(value);
  _error = Wire.endTransmission();

  if (_error == 0) {
    return true;
  }

  return false;
}

bool plugin_015_readUInt(unsigned char address, unsigned int& value)

// Reads an unsigned integer (16 bits) from a TSL2561 address (low byte first)
// Address: TSL2561 address (0 to 15), low byte first
// Value will be set to stored unsigned integer
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() above)
{
  // Set up command byte for read
  Wire.beginTransmission(plugin_015_i2caddr);
  Wire.write((address & 0x0F) | TSL2561_CMD);
  _error = Wire.endTransmission();

  // Read two bytes (low and high)
  if (_error == 0)
  {
    Wire.requestFrom(plugin_015_i2caddr, (byte)2);

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

bool plugin_015_writeUInt(unsigned char address, unsigned int value)

// Write an unsigned integer (16 bits) to a TSL2561 address (low byte first)
// Address: TSL2561 address (0 to 15), low byte first
// Value: unsigned int to write to address
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() above)
{
  // Split int into lower and upper bytes, write each byte
  if (plugin_015_writeByte(address, lowByte(value))
      && plugin_015_writeByte(address + 1, highByte(value))) {
    return true;
  }

  return false;
}

bool plugin_015_setTiming(bool gain, unsigned char time)

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
  if (plugin_015_readByte(TSL2561_REG_TIMING, timing))
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
    if (plugin_015_writeByte(TSL2561_REG_TIMING, timing)) {
      return true;
    }
  }
  return false;
}

bool plugin_015_setTiming(bool gain, unsigned char time, float& ms)

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
    case 0: ms  = 13.7; break;
    case 1: ms  = 101; break;
    case 2: ms  = 402; break;
    default: ms = 402; // used in a division, so do not use 0
  }

  // Set integration using base function
  return plugin_015_setTiming(gain, time);
}

// Determine if either sensor saturated (max depends on clock freq. and integration time)
// If so, abandon ship (calculation will not be accurate)
bool plugin_015_ADC_saturated(unsigned char time, unsigned int value) {
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

bool plugin_015_setPowerUp(void)

// Turn on TSL2561, begin integrations
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() below)
{
  // Write 0x03 to command byte (power on)
  return plugin_015_writeByte(TSL2561_REG_CONTROL, 0x03);
}

bool plugin_015_setPowerDown(void)

// Turn off TSL2561
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() below)
{
  // Clear command byte (power off)
  return plugin_015_writeByte(TSL2561_REG_CONTROL, 0x00);
}

bool plugin_015_getData(unsigned int& data0, unsigned int& data1)

// Retrieve raw integration results
// data0 and data1 will be set to integration results
// Returns true (1) if successful, false (0) if there was an I2C error
// (Also see getError() below)
{
  // Get data0 and data1 out of result registers
  if (plugin_015_readUInt(TSL2561_REG_DATA_0, data0) && plugin_015_readUInt(TSL2561_REG_DATA_1, data1)) {
    return true;
  }

  return false;
}

void plugin_015_getLux(unsigned char gain,
                       float         ms,
                       unsigned int  CH0,
                       unsigned int  CH1,
                       double      & lux,
                       double      & infrared,
                       double      & broadband)

// Convert raw data to lux
// gain: 0 (1X) or 1 (16X), see setTiming()
// ms: integration time in ms, from setTiming() or from manual integration
// CH0, CH1: results from getData()
// lux will be set to resulting lux calculation
// returns true (1) if calculation was successful
// RETURNS false (0) AND lux = 0.0 IF EITHER SENSOR WAS SATURATED (0XFFFF)
{
  double ratio, d0, d1;

  // Convert from unsigned integer to floating point
  d0 = CH0; d1 = CH1;

  // We will need the ratio for subsequent calculations
  ratio = d1 / d0;

  // save original values
  infrared  = d1;
  broadband = d0;

  // Normalize for integration time
  d0 *= (402.0 / ms);
  d1 *= (402.0 / ms);

  // Normalize for gain
  if (!gain)
  {
    d0 *= 16;
    d1 *= 16;
  }

  // Determine lux per datasheet equations:
  if (ratio < 0.5)
  {
    lux = 0.0304 * d0 - 0.062 * d0 * pow(ratio, 1.4);
  } else if (ratio < 0.61)
  {
    lux = 0.0224 * d0 - 0.031 * d1;
  } else if (ratio < 0.80)
  {
    lux = 0.0128 * d0 - 0.0153 * d1;
  } else if (ratio < 1.30)
  {
    lux = 0.00146 * d0 - 0.00112 * d1;
  } else {
    // ratio >= 1.30
    lux = 0.0;
  }
}

boolean Plugin_015(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_015;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_015);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_015));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_015));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_015));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_015));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        /*
            String options1[3];
            options1[0] = F("0x39 - (default)");
            options1[1] = F("0x49");
            options1[2] = F("0x29");
         */
        int optionValues[3];
        optionValues[0] = TSL2561_ADDR;
        optionValues[1] = TSL2561_ADDR_1;
        optionValues[2] = TSL2561_ADDR_0;
        addFormSelectorI2C(F("p015_tsl2561_i2c"), 3, optionValues, PCONFIG(0));
      }


      {
        #define TSL2561_INTEGRATION_OPTION 3
        String options[TSL2561_INTEGRATION_OPTION];
        int    optionValues[TSL2561_INTEGRATION_OPTION];
        optionValues[0] = 0x00;
        options[0]      = F("13.7 ms");
        optionValues[1] = 0x01;
        options[1]      = F("101 ms");
        optionValues[2] = 0x02;
        options[2]      = F("402 ms");
        addFormSelector(F("Integration time"), F("p015_integration"), TSL2561_INTEGRATION_OPTION, options, optionValues, PCONFIG(1));
      }

      addFormCheckBox(F("Send sensor to sleep:"), F("p015_sleep"),
                      PCONFIG(2));

      {
        #define TSL2561_GAIN_OPTION 4
        String options[TSL2561_GAIN_OPTION];
        int    optionValues[TSL2561_GAIN_OPTION];
        optionValues[0] = P015_NO_GAIN;
        options[0]      = F("No Gain");
        optionValues[1] = P015_16X_GAIN;
        options[1]      = F("16x Gain");
        optionValues[2] = P015_AUTO_GAIN;
        options[2]      = F("Auto Gain");
        optionValues[3] = P015_EXT_AUTO_GAIN;
        options[3]      = F("Extended Auto Gain");
        addFormSelector(F("Gain"), F("p015_gain"), TSL2561_GAIN_OPTION, options, optionValues, PCONFIG(3));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("p015_tsl2561_i2c"));
      PCONFIG(1) = getFormItemInt(F("p015_integration"));
      PCONFIG(2) = isFormItemChecked(F("p015_sleep"));
      PCONFIG(3) = getFormItemInt(F("p015_gain"));

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      plugin_015_i2caddr = PCONFIG(0);

      unsigned int gain; // Gain setting, 0 = X1, 1 = X16, 2 = auto, 3 = extended auto;
      float ms;          // Integration ("shutter") time in milliseconds

      plugin_015_begin();

      // If gain = false (0), device is set to low gain (1X)
      // If gain = high (1), device is set to high gain (16X)
      gain = PCONFIG(3);
      const bool autoGain       = gain == P015_AUTO_GAIN || gain == P015_EXT_AUTO_GAIN;
      static bool gain16xActive = gain == 1; // FIXME TD-er: Do not use static, it may affect multiple instances of this plugin

      if (!autoGain) {
        gain16xActive = gain == 1;
      }

      int attempt = autoGain ? 2 : 1;

      while (!success && attempt > 0) {
        --attempt;

        // If time = 0, integration will be 13.7ms
        // If time = 1, integration will be 101ms
        // If time = 2, integration will be 402ms
        unsigned char time = PCONFIG(1);
        plugin_015_setTiming(gain16xActive, time, ms);
        plugin_015_setPowerUp();
        delayBackground(ms); // FIXME TD-er: Do not use delayBackground but collect data later.
        unsigned int data0, data1;

        if (plugin_015_getData(data0, data1))
        {
          double lux;       // Resulting lux value
          double infrared;  // Resulting infrared value
          double broadband; // Resulting broadband value


          // Perform lux calculation:
          success = !plugin_015_ADC_saturated(time,  data0) && !plugin_015_ADC_saturated(time, data1);
          plugin_015_getLux(gain16xActive, ms, data0, data1, lux, infrared, broadband);

          if (autoGain) {
            if (gain16xActive) {
              // Last reading was using 16x gain
              // Check using some margin to see if gain is still needed
              if (plugin_015_ADC_saturated(time,  data0 * 16)) {
                gain16xActive = false;
              }
            } else {
              // Check using some margin to see if gain will improve reading resolution
              if (lux < 40) {
                gain16xActive = true;
              }
            }
          }

          if (success) {
            if (broadband > 0.0) {
              // Store the ratio in an unused user var. (should we make it available?)
              // Only store/update it when not close to the limits of both ADC ranges.
              // When using this value to compute extended ranges, it must not be using a ratio taken from a
              // heated sensor, since then the IR part may be off quite a bit resulting in very unrealistic values.
              if (!plugin_015_ADC_saturated(time,  data0 * 2) && !plugin_015_ADC_saturated(time, data1 * 2)) {
                UserVar[event->BaseVarIndex + 3] = infrared / broadband;
              }
            }
          } else {
            // Use last known ratio to reconstruct the broadband value
            // If IR is saturated, output the max value based on the last known ratio.
            if ((UserVar[event->BaseVarIndex + 3] > 0.0) && (gain == P015_EXT_AUTO_GAIN)) {
              data0 = static_cast<double>(data1) / UserVar[event->BaseVarIndex + 3];
              plugin_015_getLux(gain16xActive, ms, data0, data1, lux, infrared, broadband);
              success = true;
            }
          }
          UserVar[event->BaseVarIndex]     = lux;
          UserVar[event->BaseVarIndex + 1] = infrared;
          UserVar[event->BaseVarIndex + 2] = broadband;

          if (!success)
          {
            addLog(LOG_LEVEL_INFO, F("TSL2561: Sensor saturated!"));
            break;
          }

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("TSL2561: Address: 0x");
            log += String(plugin_015_i2caddr, HEX);
            log += F(": Mode: ");
            log += String(time, HEX);
            log += F(": Gain: ");
            log += String(gain, HEX);
            log += F(": Lux: ");
            log += UserVar[event->BaseVarIndex];
            log += F(": Infrared: ");
            log += UserVar[event->BaseVarIndex + 1];
            log += F(": Broadband: ");
            log += UserVar[event->BaseVarIndex + 2];
            log += F(": Ratio: ");
            log += UserVar[event->BaseVarIndex + 3];
            addLog(LOG_LEVEL_INFO, log);
          }
        }
        else
        {
          // getData() returned false because of an I2C error, inform the user.
          addLog(LOG_LEVEL_ERROR, F("TSL2561: i2c error"));
          success = false;
          attempt = 0;
        }
      }

      if (PCONFIG(2)) {
        addLog(LOG_LEVEL_DEBUG_MORE, F("TSL2561: sleeping..."));
        plugin_015_setPowerDown();
      }

      break;
    }
  }
  return success;
}

#endif // USES_P015
