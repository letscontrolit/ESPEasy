//#######################################################################################################
//#################### Plugin 037 Moisture & Temperature & Light I2C Soil moisture sensor  ##############
//#######################################################################################################
//
// Capacitive soil moisture sensor
// like this one: https://www.tindie.com/products/miceuz/i2c-soil-moisture-sensor/
// based on this library: https://github.com/Apollon77/I2CSoilMoistureSensor
//
#define PLUGIN_037
#define PLUGIN_ID_037        37
#define PLUGIN_NAME_037       "Moisture & Temperature & Light I2C Soil moisture sensor [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_037 "Temperature"
#define PLUGIN_VALUENAME2_037 "Moisture"
#define PLUGIN_VALUENAME3_037 "Light"


//Default I2C Address of the sensor
#define SOILMOISTURESENSOR_DEFAULT_ADDR 0x20

//Soil Moisture Sensor Register Addresses
#define SOILMOISTURESENSOR_GET_CAPACITANCE 	0x00 // (r) 	2 bytes
#define SOILMOISTURESENSOR_SET_ADDRESS 		0x01 //	(w) 	1 byte
#define SOILMOISTURESENSOR_GET_ADDRESS 		0x02 // (r) 	1 byte
#define SOILMOISTURESENSOR_MEASURE_LIGHT 	0x03 //	(w) 	n/a
#define SOILMOISTURESENSOR_GET_LIGHT 		0x04 //	(r) 	2 bytes
#define SOILMOISTURESENSOR_GET_TEMPERATURE	0x05 //	(r) 	2 bytes
#define SOILMOISTURESENSOR_RESET 			0x06 //	(w) 	n/a
#define SOILMOISTURESENSOR_GET_VERSION 		0x07 //	(r) 	1 bytes
#define SOILMOISTURESENSOR_SLEEP	        0x08 // (w)     n/a
#define SOILMOISTURESENSOR_GET_BUSY	        0x09 // (r)	    1 bytes





boolean Plugin_037_init[2] = {false, false};

boolean Plugin_037(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_037;
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
        string = F(PLUGIN_NAME_037);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_037));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_037));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_037));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        string += F("<TR><TD>I2C Address: (decimal, no HEX!)<TD><input type='text' title='Set i2c Address of sensor' name='");
        string += F("plugin_037_i2cSoilMoisture_i2cAddress' value='");
        string += Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        string += F("'>");
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_037_i2cSoilMoisture_i2cAddress");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        uint8_t idx = Settings.TaskDevicePluginConfig[event->TaskIndex][0] & 0x1; //Addresses are 0x76 and 0x77 so we may use it this way
        uint8_t a = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        _i2caddr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

        // Plugin_037_write8(SOILMOISTURESENSOR_RESET, 0x00);
        // delay(1000);

        UserVar[event->BaseVarIndex] = ((float)Plugin_037_readTemperature(idx)) / 10;
        UserVar[event->BaseVarIndex + 1] = ((float)Plugin_037_readMoisture(idx));
        UserVar[event->BaseVarIndex + 2] = ((float)Plugin_037_readLight(idx));
        // send sensor to sleep
        Plugin_037_write8(SOILMOISTURESENSOR_SLEEP, SOILMOISTURESENSOR_SLEEP);

        String log = F("SoilMoisture  : Address: 0x");
        log += String(_i2caddr,HEX);
        addLog(LOG_LEVEL_INFO, log);
        log = F("SoilMoisture  : Temperature: ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO, log);
        log = F("SoilMoisture  : Moisture: ");
        log += UserVar[event->BaseVarIndex + 1];
        addLog(LOG_LEVEL_INFO, log);
        log = F("SoilMoisture  : Light: ");
        log += UserVar[event->BaseVarIndex + 2];
        addLog(LOG_LEVEL_INFO, log);
        success = true;

        break;
      }

  }
  return success;
}


//**************************************************************************/
// Writes an 8 bit value over I2C/SPI
//**************************************************************************/
void Plugin_037_write8(byte reg, byte value)
{
  Wire.beginTransmission((uint8_t)_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  Wire.endTransmission();
}

//**************************************************************************/
// Reads an 8 bit value over I2C
//**************************************************************************/
uint8_t Plugin_037_read8(byte reg, bool * is_ok)
{
  uint8_t value;

  Wire.beginTransmission((uint8_t)_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  byte count = Wire.requestFrom((uint8_t)_i2caddr, (byte)1);
  if (is_ok != NULL) { *is_ok = (count == 1); }
  value = Wire.read();
  Wire.endTransmission();
  return value;
}

//**************************************************************************/
// Reads a 16 bit value over I2C
//**************************************************************************/
uint16_t Plugin_037_read16(byte reg)
{
  uint16_t value;

  Wire.beginTransmission((uint8_t)_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)_i2caddr, (byte)2);
  value = (Wire.read() << 8) | Wire.read();
  Wire.endTransmission();

  return value;
}


//**************************************************************************/
// Reads a signed 16 bit value over I2C
//**************************************************************************/
int16_t Plugin_037_readS16(byte reg)
{
  return (int16_t)Plugin_037_read16(reg);
}


//**************************************************************************/
// Read temperature
//**************************************************************************/
float Plugin_037_readTemperature(uint8_t idx)
{
  return Plugin_037_readS16(SOILMOISTURESENSOR_GET_TEMPERATURE);
}

//**************************************************************************/
// Read light
//**************************************************************************/
float Plugin_037_readLight(uint8_t idx) {
  Plugin_037_write8(SOILMOISTURESENSOR_MEASURE_LIGHT,SOILMOISTURESENSOR_MEASURE_LIGHT);
  // delay(2000);  //we need this delay, otherwise we get only the last reading...
  return Plugin_037_read16(SOILMOISTURESENSOR_GET_LIGHT);
}

//**************************************************************************/
// Read moisture
//**************************************************************************/
unsigned int Plugin_037_readMoisture(uint8_t idx) {
  return Plugin_037_read16(SOILMOISTURESENSOR_GET_CAPACITANCE);
}
