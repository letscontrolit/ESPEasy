#include "_Plugin_Helper.h"
#ifdef USES_P154

// #######################################################################################################
// #################################### Plugin-154: Environment - BMP3xx   ###############################
// #######################################################################################################

# include "src/PluginStructs/P154_data_struct.h"

# define PLUGIN_154
# define PLUGIN_ID_154         154
# define PLUGIN_NAME_154       "Environment - BMP3xx"
# define PLUGIN_VALUENAME1_154 "Temperature"
# define PLUGIN_VALUENAME2_154 "Pressure"

boolean Plugin_154(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_154;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TEMP_BARO;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_154);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_154));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_154));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x76, 0x77 };
      constexpr int nrAddressOptions   = NR_ELEMENTS(i2cAddressValues);

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), nrAddressOptions, i2cAddressValues, P154_I2C_ADDR);
        addFormNote(F("SDO Low=0x76, High=0x77"));
      } else {
        success = intArrayContains(nrAddressOptions, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P154_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P154_I2C_ADDR = 0x77;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P154_data_struct(event));
      P154_data_struct *P154_data =
        static_cast<P154_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P154_data && P154_data->begin());
      break;
    }

    case PLUGIN_READ:
    {
      P154_data_struct *P154_data =
        static_cast<P154_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P154_data) {
        break;
      }

      success = P154_data->read(UserVar[event->BaseVarIndex], UserVar[event->BaseVarIndex + 1]);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = P154_data_struct::webformLoad(event);
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      success = P154_data_struct::webformSave(event);
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P154
