#include "_Plugin_Helper.h"
#ifdef USES_P007

// #######################################################################################################
// #################################### Plugin 007: ExtWiredAnalog #######################################
// #######################################################################################################


# define PLUGIN_007
# define PLUGIN_ID_007         7
# define PLUGIN_NAME_007       "Analog input - PCF8591"
# define PLUGIN_VALUENAME1_007 "Analog"

boolean Plugin_007(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_007;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_007);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_007));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      String  portNames[4];
      int     portValues[4];
      uint8_t unit    = (CONFIG_PORT - 1) / 4;
      uint8_t port    = CONFIG_PORT - (unit * 4);
      uint8_t address = 0x48 + unit;

      for (uint8_t x = 0; x < 4; x++) {
        portValues[x] = x + 1;
        portNames[x]  = 'A';
        portNames[x] += x;
      }
      uint8_t optionValues[8] = { 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f };
      addFormSelectorI2C(F("plugin_007_i2c"), 8, optionValues, address);
      addFormSelector(F("Port"), F("plugin_007_port"), 4, portNames, portValues, port);

      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      uint8_t i2c  = getFormItemInt(F("plugin_007_i2c"));
      uint8_t port = getFormItemInt(F("plugin_007_port"));
      CONFIG_PORT = (((i2c - 0x48) << 2) + port);

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = true;
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      const int i2cAddressValues[] = { 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f };
      success = intArrayContains(8, i2cAddressValues, event->Par1);
      break;
    }

    case PLUGIN_READ:
    {
      uint8_t unit    = (CONFIG_PORT - 1) / 4;
      uint8_t port    = CONFIG_PORT - (unit * 4);
      uint8_t address = 0x48 + unit;

      // get the current pin value
      Wire.beginTransmission(address);
      Wire.write(port - 1);
      Wire.endTransmission();

      Wire.requestFrom(address, (uint8_t)0x2);

      if (Wire.available())
      {
        Wire.read();                                       // Read older value first (stored in chip)
        UserVar[event->BaseVarIndex] = (float)Wire.read(); // now read actual value and store into Nodo var

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          log.reserve(30);
          log = F("PCF  : Analog value: ");
          log += formatUserVarNoCheck(event->TaskIndex, 0);
          addLog(LOG_LEVEL_INFO, log);
        }
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P007
