#include "_Plugin_Helper.h"
#ifdef USES_P126

// #######################################################################################################
// ################################ Plugin 126: Irms through ADS1015 I2C #################################
// #######################################################################################################


#include "src/PluginStructs/P126_data_struct.h"

#define PLUGIN_126
#define PLUGIN_ID_126 126
#define PLUGIN_NAME_126 "Current Irms Sensor (ADS1015)"
#define PLUGIN_VALUENAME1_126 "IrmsADS1015_Canal1"
#define PLUGIN_VALUENAME2_126 "IrmsADS1015_Canal2"


boolean Plugin_126(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  // static uint8_t portValue = 0;
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_126;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_DUAL;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_126);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_126));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_126));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      #define ADS1115_I2C_OPTION 4
      const uint8_t i2cAddressValues[] = { 0x48, 0x49, 0x4A, 0x4B };
      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), ADS1115_I2C_OPTION, i2cAddressValues, PCONFIG(0));
      } else {
        success = intArrayContains(ADS1115_I2C_OPTION, i2cAddressValues, event->Par1);
      }
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Calibration"));
      addFormNumericBox(F("Canal 1"), F("P126_adc01"), PCONFIG_LONG(0), 0, 240);
      addFormNumericBox(F("Canal 2"), F("P126_adc23"), PCONFIG_LONG(1), 0, 240);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));
      PCONFIG_LONG(0)  = getFormItemInt(F("P126_adc01"));
      PCONFIG_LONG(1)  = getFormItemInt(F("P126_adc23"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      const uint8_t address = PCONFIG(0);
      const int c1_calibre = PCONFIG_LONG(0);
      const int c2_calibre = PCONFIG_LONG(1);

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P126_data_struct(address, c1_calibre, c2_calibre));
      P126_data_struct *P126_data =
        static_cast<P126_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P126_data) {
        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {
      P126_data_struct *P126_data =
        static_cast<P126_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P126_data) {
        const bool res_read = P126_data->read();
        if ( true != res_read ) {
          if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
            addLog(LOG_LEVEL_ERROR, F("IRMS: failed reading values"));
          }
        } else {
          const float c1Current = P126_data->getCurrent(1);
          const float c2Current = P126_data->getCurrent(2);
          UserVar[event->BaseVarIndex] = c1Current;
          UserVar[event->BaseVarIndex+1] = c2Current;
          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            String log = "IRMS: Reading canal1 current = ";
            log += c1Current;
            log += " / canal2 current = ";
            log += c2Current;
            addLog(LOG_LEVEL_DEBUG, log);
          }
          success = true;
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P126
