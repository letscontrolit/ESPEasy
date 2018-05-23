#ifdef USES_P068
//#######################################################################################################
//################ Plugin 68: SHT30/SHT31/SHT35 Temperature and Humidity Sensor (I2C) ###################
//#######################################################################################################
//######################## Library source code for Arduino by WeMos, 2016 ###############################
//#######################################################################################################
//###################### Plugin for ESP Easy by B.E.I.C. ELECTRONICS, 2017 ##############################
//############################### http://www.beicelectronics.com ########################################
//#######################################################################################################
//########################## Adapted to ESPEasy 2.0 by Jochen Krapf #####################################
//#######################################################################################################


#define PLUGIN_068
#define PLUGIN_ID_068         68
#define PLUGIN_NAME_068       "Environment - SHT30/31/35 [TESTING]"
#define PLUGIN_VALUENAME1_068 "Temperature"
#define PLUGIN_VALUENAME2_068 "Humidity"

//==============================================
// SHT3X LIBRARY - SHT3X.h
// =============================================
# ifndef SHT3X_H
# define SHT3X_H

class SHT3X
{
public:
	SHT3X(uint8_t addr);
	void get(void);
	float tmp=0;
	float hum=0;

private:
	uint8_t _i2c_device_address;
};

#endif

//==============================================
// SHT3X LIBRARY - SHT3X.cpp
// =============================================
SHT3X::SHT3X(uint8_t addr)
{
	_i2c_device_address = addr;
	//Wire.begin();   called in ESPEasy framework

	// Set to periodic mode
	Wire.beginTransmission(_i2c_device_address);
	Wire.write(0x20);   // periodic 0.5mps
	Wire.write(0x32);   // repeatability high
	Wire.endTransmission();
}

void SHT3X::get()
{
	uint16_t data[6];

	Wire.beginTransmission(_i2c_device_address);
	Wire.write(0xE0);   // fetch data command
	Wire.write(0x00);
	Wire.endTransmission();

	Wire.requestFrom(_i2c_device_address, (uint8_t)6);
	if (Wire.available() == 6)
	{
		data[0] = Wire.read();
		data[1] = Wire.read();
		data[2] = Wire.read();
		data[3] = Wire.read();
		data[4] = Wire.read();
		data[5] = Wire.read();

		//TODO: check CRC

		tmp = ((((data[0] << 8) | data[1]) * 175.0) / 65535.0) - 45.0;
		hum = ((((data[3] << 8) | data[4]) * 100.0) / 65535.0);
	}
	else
	{
		tmp = NAN;
		hum = NAN;
	}
}

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif

SHT3X*  Plugin_068_SHT3x[TASKS_MAX] = { NULL, };


//==============================================
// PLUGIN
// =============================================

boolean Plugin_068(byte function, struct EventStruct *event, String& string)
{
	boolean success = false;

	switch (function)
	{
		case PLUGIN_DEVICE_ADD:
		{
			Device[++deviceCount].Number = PLUGIN_ID_068;
			Device[deviceCount].Type = DEVICE_TYPE_I2C;
			Device[deviceCount].VType = SENSOR_TYPE_TEMP_HUM;
			Device[deviceCount].Ports = 0;
			Device[deviceCount].PullUpOption = false;
			Device[deviceCount].InverseLogicOption = false;
			Device[deviceCount].FormulaOption = true;
			Device[deviceCount].ValueCount = 2;
			Device[deviceCount].SendDataOption = true;
			Device[deviceCount].TimerOption = true;
			Device[deviceCount].GlobalSyncOption = true;
			break;
		}

		case PLUGIN_GET_DEVICENAME:
		{
			string = F(PLUGIN_NAME_068);
			break;
		}

		case PLUGIN_GET_DEVICEVALUENAMES:
		{
			strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_068));
			strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_068));
			break;
		}

		case PLUGIN_WEBFORM_LOAD:
		{
			int optionValues[2] = { 0x44, 0x45 };
			addFormSelectorI2C(F("i2c_addr"), 2, optionValues, CONFIG(0));

			success = true;
			break;
		}

		case PLUGIN_WEBFORM_SAVE:
		{
			CONFIG(0) = getFormItemInt(F("i2c_addr"));

			success = true;
			break;
		}

		case PLUGIN_INIT:
		{
			if (Plugin_068_SHT3x[event->TaskIndex])
				delete Plugin_068_SHT3x[event->TaskIndex];
			Plugin_068_SHT3x[event->TaskIndex] = new SHT3X(CONFIG(0));

			success = true;
			break;
		}

		case PLUGIN_READ:
		{
			if (!Plugin_068_SHT3x[event->TaskIndex])
				return success;

			Plugin_068_SHT3x[event->TaskIndex]->get();
			UserVar[event->BaseVarIndex + 0] = Plugin_068_SHT3x[event->TaskIndex]->tmp;
			UserVar[event->BaseVarIndex + 1] = Plugin_068_SHT3x[event->TaskIndex]->hum;
			String log = F("SHT3x: Temperature: ");
			log += UserVar[event->BaseVarIndex + 0];
			addLog(LOG_LEVEL_INFO, log);
			log = F("SHT3x: Humidity: ");
			log += UserVar[event->BaseVarIndex + 1];
			addLog(LOG_LEVEL_INFO, log);
			success = true;
			break;
		}
	}
	return success;
}

#endif // USES_P068
