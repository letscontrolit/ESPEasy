// #######################################################################################################
// ######################## Plugin 127 CDM7160 I2C CO2 Sensor ############################################
// #######################################################################################################
// development version
// by: V0JT4
// Resolves: https://github.com/letscontrolit/ESPEasy/issues/986
//
// Changelog:
// 2022-06-24, tonhuisman Remove delay from init call, optimize some code
// 2022-01-13, tonhuisman Ignore measured values > 15000: unit is still initializing
//                        Change status from Development to Testing
// 2021-12-31, tonhuisman Migrate plugin from ESPEasyPluginPlayground to ESPEasy repository
// - Restructured: Use ESPEasy 'modern' code
// - Restructured: Split into PluginDataStruct
// - Select Plugin ID 127

#include "_Plugin_Helper.h"
#ifdef USES_P127

# include "src/PluginStructs/P127_data_struct.h"

# define PLUGIN_127
# define PLUGIN_ID_127         127
# define PLUGIN_NAME_127       "Gases - CO2 CDM7160"
# define PLUGIN_VALUENAME1_127 "CO2"


boolean Plugin_127_init = false;


boolean Plugin_127(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_127;
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
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_127);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_127));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { CDM7160_ADDR, CDM7160_ADDR_0 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, P127_CONFIG_I2C_ADDRESS);
        # ifndef LIMIT_BUILD_SIZE
        addFormNote(F("CAD0 High/open=0x69, Low=0x68"));
        # endif // ifndef LIMIT_BUILD_SIZE
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P127_CONFIG_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0; // No decimals needed
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Altitude"), F("altitude"), P127_CONFIG_ALTITUDE, 0, 2550); // Max. 2550 meter supported
      addUnit('m');

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P127_CONFIG_I2C_ADDRESS = getFormItemInt(F("i2c_addr"));
      P127_CONFIG_ALTITUDE    = getFormItemInt(F("altitude"));

      success = true;
      break;
    }
    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P127_data_struct(P127_CONFIG_I2C_ADDRESS, P127_CONFIG_ALTITUDE));
      P127_data_struct *P127_data = static_cast<P127_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P127_data) && P127_data->init();

      break;
    }
    case PLUGIN_ONCE_A_SECOND:
    {
      P127_data_struct *P127_data = static_cast<P127_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P127_data) {
        success = P127_data->checkData();
      }

      break;
    }
    case PLUGIN_READ:
    {
      P127_data_struct *P127_data = static_cast<P127_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P127_data) {
        return success;
      }

      UserVar[event->BaseVarIndex] = P127_data->readData();

      success = true;

      if (UserVar[event->BaseVarIndex] > 15000) {
        addLog(LOG_LEVEL_ERROR, F("CDM7160: Sensor still initializing, data ignored"));
        success = false; // Do not send out to controllers
      } else if (UserVar[event->BaseVarIndex] > 10000) {
        addLog(LOG_LEVEL_ERROR, F("CDM7160: Sensor saturated! > 10000 ppm"));
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("CDM7160: Address: 0x");
        log += String(P127_CONFIG_I2C_ADDRESS, HEX);
        log += F(": CO2 ppm: ");
        log += UserVar[event->BaseVarIndex];
        log += F(", alt: ");
        log += P127_data->getAltitude();
        log += F(", comp: ");
        log += P127_data->getCompensation();
        addLogMove(LOG_LEVEL_INFO, log);
      }
      break;
    }
    case PLUGIN_WRITE:
    {
      P127_data_struct *P127_data = static_cast<P127_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P127_data) {
        String command = parseString(string, 1);

        if (equals(command, F("cdmrst"))) {
          addLog(LOG_LEVEL_INFO, F("CDM7160: reset"));
          P127_data->setReset();
          success = true;
        }
      }

      break;
    }

    case PLUGIN_FIFTY_PER_SECOND: // Handle delays
    {
      P127_data_struct *P127_data = static_cast<P127_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P127_data) {
        success = P127_data->plugin_fifty_per_second();
      }

      break;
    }
  }
  return success;
}

#endif // USES_P127
