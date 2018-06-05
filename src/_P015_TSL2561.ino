#ifdef USES_P015
//#######################################################################################################
//######################## Plugin 015 TSL2561 I2C Lux Sensor ############################################
//#######################################################################################################
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

boolean Plugin_015_init = false;


#define TSL2561_ADDR_0 0x29 // address with '0' shorted on board
#define TSL2561_ADDR   0x39 // default address
#define TSL2561_ADDR_1 0x49 // address with '1' shorted on board

#define TSL2561_CMD           0x80
#define	TSL2561_REG_CONTROL   0x00
#define	TSL2561_REG_TIMING    0x01
#define	TSL2561_REG_DATA_0    0x0C
#define	TSL2561_REG_DATA_1    0x0E


byte plugin_015_i2caddr;
byte _error;

boolean plugin_015_begin()
{
	//Wire.begin();   called in ESPEasy framework
	return(true);
}

boolean plugin_015_readByte(unsigned char address, unsigned char &value)
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
		Wire.requestFrom(plugin_015_i2caddr,(byte)1);
		if (Wire.available() == 1)
		{
			value = Wire.read();
			return(true);
		}
	}
	return(false);
}

boolean plugin_015_writeByte(unsigned char address, unsigned char value)
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
	if (_error == 0)
		return(true);

	return(false);
}


boolean plugin_015_readUInt(unsigned char address, unsigned int &value)
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
		Wire.requestFrom(plugin_015_i2caddr,(byte)2);
		if (Wire.available() == 2)
		{
			char high, low;
			low = Wire.read();
			high = Wire.read();
			// Combine bytes into unsigned int
			value = word(high,low);
			return(true);
		}
	}
	return(false);
}


boolean plugin_015_writeUInt(unsigned char address, unsigned int value)
	// Write an unsigned integer (16 bits) to a TSL2561 address (low byte first)
	// Address: TSL2561 address (0 to 15), low byte first
	// Value: unsigned int to write to address
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() above)
{
	// Split int into lower and upper bytes, write each byte
	if (plugin_015_writeByte(address,lowByte(value))
		&& plugin_015_writeByte(address + 1,highByte(value)))
		return(true);

	return(false);
}



boolean plugin_015_setTiming(boolean gain, unsigned char time)
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
	if (plugin_015_readByte(TSL2561_REG_TIMING,timing))
	{
		// Set gain (0 or 1)
		if (gain)
			timing |= 0x10;
		else
			timing &= ~0x10;

		// Set integration time (0 to 3)
		timing &= ~0x03;
		timing |= (time & 0x03);

		// Write modified timing byte back to device
		if (plugin_015_writeByte(TSL2561_REG_TIMING,timing))
			return(true);
	}
	return(false);
}


boolean plugin_015_setTiming(boolean gain, unsigned char time, unsigned int &ms)
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
		case 0: ms = 14; break;
		case 1: ms = 101; break;
		case 2: ms = 402; break;
		default: ms = 0;
	}
	// Set integration using base function
	return(plugin_015_setTiming(gain,time));
}


boolean plugin_015_setPowerUp(void)
	// Turn on TSL2561, begin integrations
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	// Write 0x03 to command byte (power on)
	return(plugin_015_writeByte(TSL2561_REG_CONTROL,0x03));
}


boolean plugin_015_setPowerDown(void)
	// Turn off TSL2561
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	// Clear command byte (power off)
	return(plugin_015_writeByte(TSL2561_REG_CONTROL,0x00));
}

boolean plugin_015_getData(unsigned int &data0, unsigned int &data1)
	// Retrieve raw integration results
	// data0 and data1 will be set to integration results
	// Returns true (1) if successful, false (0) if there was an I2C error
	// (Also see getError() below)
{
	// Get data0 and data1 out of result registers
	if (plugin_015_readUInt(TSL2561_REG_DATA_0,data0) && plugin_015_readUInt(TSL2561_REG_DATA_1,data1))
		return(true);

	return(false);
}


boolean plugin_015_getLux(unsigned char gain, unsigned int ms, unsigned int CH0, unsigned int CH1, double &lux, double &infrared, double &broadband)
	// Convert raw data to lux
	// gain: 0 (1X) or 1 (16X), see setTiming()
	// ms: integration time in ms, from setTiming() or from manual integration
	// CH0, CH1: results from getData()
	// lux will be set to resulting lux calculation
	// returns true (1) if calculation was successful
	// RETURNS false (0) AND lux = 0.0 IF EITHER SENSOR WAS SATURATED (0XFFFF)
{


	// Determine if either sensor saturated (0xFFFF)
	// If so, abandon ship (calculation will not be accurate)
	if ((CH0 == 0xFFFF) || (CH1 == 0xFFFF))
	{
		lux = 65535.0;
		return(false);
	}
	else
	{
		double ratio, d0, d1;
		// Convert from unsigned integer to floating point
		d0 = CH0; d1 = CH1;

		// We will need the ratio for subsequent calculations
		ratio = d1 / d0;

    // save original values
    infrared = d1;
    broadband = d0;

		// Normalize for integration time
		d0 *= (402.0/ms);
		d1 *= (402.0/ms);

		// Normalize for gain
		if (!gain)
		{
			d0 *= 16;
			d1 *= 16;
		}

		// Determine lux per datasheet equations:

		if (ratio < 0.5)
		{
			lux = 0.0304 * d0 - 0.062 * d0 * pow(ratio,1.4);
			return(true);
		}

		if (ratio < 0.61)
		{
			lux = 0.0224 * d0 - 0.031 * d1;
			return(true);
		}

		if (ratio < 0.80)
		{
			lux = 0.0128 * d0 - 0.0153 * d1;
			return(true);
		}

		if (ratio < 1.30)
		{
			lux = 0.00146 * d0 - 0.00112 * d1;
			return(true);
		}

		// if (ratio > 1.30)
		lux = 0.0;
		return(true);
	}
}


boolean Plugin_015(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_015;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_TRIPLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
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

        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice1 = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
				/*
        String options1[3];
        options1[0] = F("0x39 - (default)");
        options1[1] = F("0x49");
        options1[2] = F("0x29");
				*/
        int optionValues1[3];
        optionValues1[0] = TSL2561_ADDR;
        optionValues1[1] = TSL2561_ADDR_1;
        optionValues1[2] = TSL2561_ADDR_0;
				addFormSelectorI2C(F("plugin_015_tsl2561_i2c"), 3, optionValues1, choice1);

        #define TSL2561_INTEGRATION_OPTION 3

        byte choice2 = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        String options2[TSL2561_INTEGRATION_OPTION];
        int optionValues2[TSL2561_INTEGRATION_OPTION];
        optionValues2[0] = 0x00;
        options2[0] = F("13 ms");
        optionValues2[1] = 0x01;
        options2[1] = F("101 ms");
        optionValues2[2] = 0x02;
        options2[2] = F("402 ms");
				addFormSelector(F("Integration time"), F("plugin_015_integration"), TSL2561_INTEGRATION_OPTION, options2, optionValues2, choice2);

        addFormCheckBox(F("Send sensor to sleep:"), F("plugin_015_sleep"),
        		Settings.TaskDevicePluginConfig[event->TaskIndex][2]);

        addFormCheckBox(F("Enable 16x Gain:"), F("plugin_015_gain"),
        		Settings.TaskDevicePluginConfig[event->TaskIndex][3]);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_015_tsl2561_i2c"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_015_integration"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = isFormItemChecked(F("plugin_015_sleep"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = isFormItemChecked(F("plugin_015_gain"));

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
      	plugin_015_i2caddr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

        boolean gain;     // Gain setting, 0 = X1, 1 = X16;
        unsigned int ms;  // Integration ("shutter") time in milliseconds

        plugin_015_begin();

         // If gain = false (0), device is set to low gain (1X)
         // If gain = high (1), device is set to high gain (16X)
         gain = Settings.TaskDevicePluginConfig[event->TaskIndex][3];

         // If time = 0, integration will be 13.7ms
         // If time = 1, integration will be 101ms
         // If time = 2, integration will be 402ms
         unsigned char time = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
         plugin_015_setTiming(gain,time,ms);
         plugin_015_setPowerUp();
         delayBackground(ms);
         unsigned int data0, data1;

         if (plugin_015_getData(data0,data1))
         {

           double lux;    // Resulting lux value
           double infrared;    // Resulting infrared value
           double broadband;    // Resulting broadband value
           boolean good;  // True if neither sensor is saturated

           // Perform lux calculation:

           good = plugin_015_getLux(gain,ms,data0,data1,lux, infrared, broadband);
        	 UserVar[event->BaseVarIndex] = lux;
           UserVar[event->BaseVarIndex + 1] = infrared;
           UserVar[event->BaseVarIndex + 2] = broadband;

           if (!good)
           {
             addLog(LOG_LEVEL_INFO,F("TSL2561: Sensor saturated! > 65535 Lux"));
           }

           success = true;
           String log = F("TSL2561: Address: 0x");
           log += String(plugin_015_i2caddr,HEX);
           log += F(": Mode: ");
           log += String(time,HEX);
           log += F(": Gain: ");
           log += String(gain,HEX);
           log += F(": Lux: ");
           log += UserVar[event->BaseVarIndex];
           log += F(": Infrared: ");
           log += UserVar[event->BaseVarIndex + 1];
           log += F(": Broadband: ");
           log += UserVar[event->BaseVarIndex + 2];
           addLog(LOG_LEVEL_INFO,log);
         }
         else
         {
           // getData() returned false because of an I2C error, inform the user.
        	 addLog(LOG_LEVEL_ERROR, F("TSL2561: i2c error"));

         }
         if (Settings.TaskDevicePluginConfig[event->TaskIndex][2]) {
        	 addLog(LOG_LEVEL_DEBUG_MORE, F("TSL2561: sleeping..."));
        	 plugin_015_setPowerDown();
         }

        break;
      }

  }
  return success;
}
#endif // USES_P015
