//#######################################################################################################
//#################### Plugin 047 Moisture & Temperature & Light I2C Soil moisture sensor  ##############
//#######################################################################################################
//
// Capacitive soil moisture sensor
// like this one: https://www.tindie.com/products/miceuz/i2c-soil-moisture-sensor/
// based on this library: https://github.com/Apollon77/I2CSoilMoistureSensor
//
#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_047
#define PLUGIN_ID_047        47
#define PLUGIN_NAME_047       "Moisture & Temperature & Light I2C Soil moisture sensor [TEST]"
#define PLUGIN_VALUENAME1_047 "Temperature"
#define PLUGIN_VALUENAME2_047 "Moisture"
#define PLUGIN_VALUENAME3_047 "Light"


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





boolean Plugin_047_init[2] = {false, false};

boolean Plugin_047(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_047;
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
        string = F(PLUGIN_NAME_047);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_047));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_047));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_047));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        string += F("<TR><TD>I2C Address (Hex): <TD><input type='text' title='Set i2c Address of sensor' name='");
        string += F("plugin_047_i2cSoilMoisture_i2cAddress' value='0x");
        string += String(Settings.TaskDevicePluginConfig[event->TaskIndex][0],HEX);
        string += F("'>");
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_047_i2cSoilMoisture_i2cAddress");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = (int) strtol(plugin1.c_str(), 0, 16);;
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        _i2caddr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

        // get sensor version to check if sensor is present
        uint8_t sensorVersion = Plugin_047_getVersion();
        if (sensorVersion==0x22 || sensorVersion==0x23) {
          //valid sensor
        }
        else {
          addLog(LOG_LEVEL_INFO, "SoilMoisture: Bad Version, no Sensor?");
          Plugin_047_write8(SOILMOISTURESENSOR_RESET, SOILMOISTURESENSOR_RESET);
          break;
        }

        // start light measurement
        Plugin_047_write8(SOILMOISTURESENSOR_MEASURE_LIGHT,SOILMOISTURESENSOR_MEASURE_LIGHT);

        UserVar[event->BaseVarIndex] = ((float)Plugin_047_readTemperature()) / 10;
        UserVar[event->BaseVarIndex + 1] = ((float)Plugin_047_readMoisture());

        delay(1000);  //we need this delay, otherwise we get only the last reading...
        UserVar[event->BaseVarIndex + 2] = ((float)Plugin_047_readLight());
        // send sensor to sleep
        Plugin_047_write8(SOILMOISTURESENSOR_SLEEP, SOILMOISTURESENSOR_SLEEP);

        String log = F("SoilMoisture  : Address: 0x");
        log += String(_i2caddr,HEX);
        log += F(" Version: 0x");
        log += String(sensorVersion,HEX);
        addLog(LOG_LEVEL_INFO, log);
        log = F("SoilMoisture: Temperature: ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO, log);
        log = F("SoilMoisture: Moisture: ");
        log += UserVar[event->BaseVarIndex + 1];
        addLog(LOG_LEVEL_INFO, log);
        log = F("SoilMoisture: Light: ");
        log += UserVar[event->BaseVarIndex + 2];
        addLog(LOG_LEVEL_INFO, log);

        if (UserVar[event->BaseVarIndex]>100 || UserVar[event->BaseVarIndex] < -40 ||
          UserVar[event->BaseVarIndex + 1] > 800 || UserVar[event->BaseVarIndex + 1] < 1 ||
          UserVar[event->BaseVarIndex + 2] > 65535 || UserVar[event->BaseVarIndex + 2] < 0) {
            addLog(LOG_LEVEL_INFO, "SoilMoisture: Bad Reading, resetting Sensor...");
            Plugin_047_write8(SOILMOISTURESENSOR_RESET, SOILMOISTURESENSOR_RESET);
            break;
          }
        success = true;
        break;
      }

  }
  return success;
}


//**************************************************************************/
// Writes an 8 bit value over I2C/SPI
//**************************************************************************/
void Plugin_047_write8(byte reg, byte value)
{
  Wire.beginTransmission((uint8_t)_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  Wire.endTransmission();
}

//**************************************************************************/
// Reads an 8 bit value over I2C
//**************************************************************************/
uint8_t Plugin_047_read8(byte reg)
{
  uint8_t value;
  Wire.beginTransmission((uint8_t)_i2caddr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)_i2caddr, (byte)1);
  return Wire.read();
}

//**************************************************************************/
// Reads a 16 bit value over I2C
//**************************************************************************/
uint16_t Plugin_047_read16(byte reg)
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
int16_t Plugin_047_readS16(byte reg)
{
  return (int16_t)Plugin_047_read16(reg);
}


//**************************************************************************/
// Read temperature
//**************************************************************************/
float Plugin_047_readTemperature()
{
  return Plugin_047_readS16(SOILMOISTURESENSOR_GET_TEMPERATURE);
}

//**************************************************************************/
// Read light
//**************************************************************************/
float Plugin_047_readLight() {
  return Plugin_047_read16(SOILMOISTURESENSOR_GET_LIGHT);
}

//**************************************************************************/
// Read moisture
//**************************************************************************/
unsigned int Plugin_047_readMoisture() {
  return Plugin_047_read16(SOILMOISTURESENSOR_GET_CAPACITANCE);
}

// Read Sensor Version
uint8_t Plugin_047_getVersion() {
  return Plugin_047_read8(SOILMOISTURESENSOR_GET_VERSION);
}


#endif
