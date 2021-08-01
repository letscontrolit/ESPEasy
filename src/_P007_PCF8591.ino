#include "_Plugin_Helper.h"
#ifdef USES_P007

// #######################################################################################################
// #################################### Plugin 007: ExtWiredAnalog #######################################
// #######################################################################################################



#define PLUGIN_007
#define PLUGIN_ID_007         7
#define PLUGIN_NAME_007       "Analog input - PCF8591"
#define PLUGIN_VALUENAME1_007 "Analog"

boolean Plugin_007(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  // static uint8_t portValue = 0;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_007;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 4;
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

    case PLUGIN_READ:
    {
      uint8_t unit       = (CONFIG_PORT - 1) / 4;
      uint8_t port       = CONFIG_PORT - (unit * 4);
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
          String log = F("PCF  : Analog value: ");
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
