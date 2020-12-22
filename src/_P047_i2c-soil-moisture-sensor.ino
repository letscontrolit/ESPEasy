#include "_Plugin_Helper.h"
#ifdef USES_P047

// #######################################################################################################
// #################### Plugin 047 Moisture & Temperature & Light I2C Soil moisture sensor  ##############
// #######################################################################################################
//
// Capacitive soil moisture sensor
// like this one: https://www.tindie.com/products/miceuz/i2c-soil-moisture-sensor/
// based on this library: https://github.com/Apollon77/I2CSoilMoistureSensor
// this code is based on version 1.1.2 of the above library
//

#define PLUGIN_047
#define PLUGIN_ID_047        47
#define PLUGIN_NAME_047       "Environment - Soil moisture sensor [TESTING]"
#define PLUGIN_VALUENAME1_047 "Temperature"
#define PLUGIN_VALUENAME2_047 "Moisture"
#define PLUGIN_VALUENAME3_047 "Light"


// Default I2C Address of the sensor
#define SOILMOISTURESENSOR_DEFAULT_ADDR 0x20

// Soil Moisture Sensor Register Addresses
#define SOILMOISTURESENSOR_GET_CAPACITANCE      0x00 // (r)     2 bytes
#define SOILMOISTURESENSOR_SET_ADDRESS          0x01 //	(w)     1 byte
#define SOILMOISTURESENSOR_GET_ADDRESS          0x02 // (r)     1 byte
#define SOILMOISTURESENSOR_MEASURE_LIGHT        0x03 //	(w)     n/a
#define SOILMOISTURESENSOR_GET_LIGHT            0x04 //	(r)     2 bytes
#define SOILMOISTURESENSOR_GET_TEMPERATURE      0x05 //	(r)     2 bytes
#define SOILMOISTURESENSOR_RESET                0x06 //	(w)     n/a
#define SOILMOISTURESENSOR_GET_VERSION          0x07 //	(r)     1 bytes
#define SOILMOISTURESENSOR_SLEEP                0x08 // (w)     n/a
#define SOILMOISTURESENSOR_GET_BUSY             0x09 // (r)	    1 bytes

#define P047_I2C_ADDR       PCONFIG(0)
#define P047_SENSOR_SLEEP   PCONFIG(1)
#define P047_CHECK_VERSION  PCONFIG(2)
#define P047_NEW_ADDR       PCONFIG(3)
#define P047_CHANGE_ADDR    PCONFIG(4)


boolean Plugin_047(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_047;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
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

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      addFormTextBox(F("I2C Address (Hex)"), F("i2c_addr"),
                     formatToHex_decimal(P047_I2C_ADDR), 4);

      // FIXME TD-er: Why not using addFormSelectorI2C here?
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormCheckBox(F("Send sensor to sleep"), F("p047_sleep"),   P047_SENSOR_SLEEP);

      addFormCheckBox(F("Check sensor version"), F("p047_version"), P047_CHECK_VERSION);

      addFormSeparator(2);

      addFormCheckBox(F("Change Sensor address"), F("p047_changeAddr"), false);
      addFormTextBox(F("Change I2C Addr. to (Hex)"), F("p047_i2cSoilMoisture_changeAddr"),
                     formatToHex_decimal(P047_I2C_ADDR), 4);

      addFormSeparator(2);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      String plugin1 = web_server.arg(F("i2c_addr"));
      P047_I2C_ADDR = (int)strtol(plugin1.c_str(), 0, 16);

      P047_SENSOR_SLEEP = isFormItemChecked(F("p047_sleep"));

      P047_CHECK_VERSION = isFormItemChecked(F("p047_version"));

      String plugin4 = web_server.arg(F("p047_i2cSoilMoisture_changeAddr"));
      P047_NEW_ADDR = (int)strtol(plugin4.c_str(), 0, 16);

      P047_CHANGE_ADDR = isFormItemChecked(F("p047_changeAddr"));
      success          = true;
      break;
    }

    case PLUGIN_READ:
    {
      if (P047_SENSOR_SLEEP) {
        // wake sensor
        Plugin_047_getVersion(P047_I2C_ADDR);
        delayBackground(20);
        addLog(LOG_LEVEL_DEBUG, F("SoilMoisture->wake"));
      }

      uint8_t sensorVersion = 0;

      if (P047_CHECK_VERSION) {
        // get sensor version to check if sensor is present
        sensorVersion = Plugin_047_getVersion(P047_I2C_ADDR);

        if ((sensorVersion == 0x22) || (sensorVersion == 0x23) || (sensorVersion == 0x24) || (sensorVersion == 0x25) ||
            (sensorVersion == 0x26)) {
          // valid sensor
        }
        else {
          addLog(LOG_LEVEL_INFO, F("SoilMoisture: Bad Version, no Sensor?"));
          I2C_write8(P047_I2C_ADDR, SOILMOISTURESENSOR_RESET);
          break;
        }
      }

      // check if we want to change the sensor address
      if (P047_CHANGE_ADDR) {
        addLog(LOG_LEVEL_INFO, String(F("SoilMoisture: Change Address: 0x")) + String(P047_I2C_ADDR, HEX) + String(F("->0x")) +
               String(P047_NEW_ADDR, HEX));

        if (Plugin_047_setAddress(P047_I2C_ADDR, P047_NEW_ADDR)) {
          P047_I2C_ADDR = P047_NEW_ADDR;
        }
        P047_CHANGE_ADDR = false;
      }

      // start light measurement
      I2C_write8(P047_I2C_ADDR, SOILMOISTURESENSOR_MEASURE_LIGHT);

      // 2 s delay ...we need this delay, otherwise we get only the last reading...
      delayBackground(2000);

      float temperature = ((float)Plugin_047_readTemperature(P047_I2C_ADDR)) / 10;
      float moisture    = ((float)Plugin_047_readMoisture(P047_I2C_ADDR));
      float light       = ((float)Plugin_047_readLight(P047_I2C_ADDR));

      if ((temperature > 100) || (temperature < -40) || (moisture > 800) || (moisture < 1) || (light > 65535) || (light < 0)) {
        addLog(LOG_LEVEL_INFO, F("SoilMoisture: Bad Reading, resetting Sensor..."));
        I2C_write8(P047_I2C_ADDR, SOILMOISTURESENSOR_RESET);
        success = false;
        break;
      }
      else {
        UserVar[event->BaseVarIndex]     = temperature;
        UserVar[event->BaseVarIndex + 1] = moisture;
        UserVar[event->BaseVarIndex + 2] = light;

        String log = F("SoilMoisture: Address: 0x");
        log += String(P047_I2C_ADDR, HEX);

        if (P047_CHECK_VERSION) {
          log += F(" Version: 0x");
          log += String(sensorVersion, HEX);
        }
        addLog(LOG_LEVEL_INFO, log);
        log  = F("SoilMoisture: Temperature: ");
        log += temperature;
        addLog(LOG_LEVEL_INFO, log);
        log  = F("SoilMoisture: Moisture: ");
        log += moisture;
        addLog(LOG_LEVEL_INFO, log);
        log  = F("SoilMoisture: Light: ");
        log += light;
        addLog(LOG_LEVEL_INFO, log);

        if (P047_SENSOR_SLEEP) {
          // send sensor to sleep
          I2C_write8(P047_I2C_ADDR, SOILMOISTURESENSOR_SLEEP);
          addLog(LOG_LEVEL_DEBUG, F("SoilMoisture->sleep"));
        }
        success = true;
        break;
      }
    }
  }
  return success;
}

// **************************************************************************/
// Read temperature
// **************************************************************************/
float Plugin_047_readTemperature(byte i2cAddr)
{
  return I2C_readS16_reg(i2cAddr, SOILMOISTURESENSOR_GET_TEMPERATURE);
}

// **************************************************************************/
// Read light
// **************************************************************************/
float Plugin_047_readLight(byte i2cAddr) {
  return I2C_read16_reg(i2cAddr, SOILMOISTURESENSOR_GET_LIGHT);
}

// **************************************************************************/
// Read moisture
// **************************************************************************/
unsigned int Plugin_047_readMoisture(byte i2cAddr) {
  return I2C_read16_reg(i2cAddr, SOILMOISTURESENSOR_GET_CAPACITANCE);
}

// Read Sensor Version
uint8_t Plugin_047_getVersion(byte i2cAddr) {
  return I2C_read8_reg(i2cAddr, SOILMOISTURESENSOR_GET_VERSION);
}

/*----------------------------------------------------------------------*
* Change I2C address of the sensor to the provided address (1..127)    *
* and do a reset after it in order for the new address to become       *
* effective if second parameter is true.                               *
* Method returns true if the new address is set successfully on sensor.*
*----------------------------------------------------------------------*/
bool Plugin_047_setAddress(byte i2cAddr, int new_i2cAddr) {
  I2C_write8_reg(i2cAddr, SOILMOISTURESENSOR_SET_ADDRESS, new_i2cAddr);
  I2C_write8_reg(i2cAddr, SOILMOISTURESENSOR_SET_ADDRESS, new_i2cAddr);
  I2C_write8(i2cAddr, SOILMOISTURESENSOR_RESET);
  delayBackground(1000); // FIXME TD-er: Why is this blocking call needed?
  i2cAddr = new_i2cAddr;
  return I2C_read8_reg(i2cAddr, SOILMOISTURESENSOR_GET_ADDRESS) == new_i2cAddr;
}

#endif // USES_P047
