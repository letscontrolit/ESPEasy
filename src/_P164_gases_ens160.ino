#include "_Plugin_Helper.h"
#ifdef USES_P164

// #######################################################################################################
// #################################### Plugin-164: Gases - ENS16x (tvoc,eco2) ###########################
// #######################################################################################################
// P164 "GASES - ENS16x (TVOC, eCO2)"
// Plugin for ENS160 & ENS161 TVOC and eCO2 sensor with I2C interface from ScioSense
// Based upon: https://github.com/sciosense/ENS160_driver
// For documentation see 
// https://www.sciosense.com/wp-content/uploads/documents/SC-001224-DS-9-ENS160-Datasheet.pdf
//
// 2023 By flashmark
// #######################################################################################################

# include "src/PluginStructs/P164_data_struct.h"

# define PLUGIN_164
# define PLUGIN_ID_164         164
# define PLUGIN_NAME_164       "Gases - ENS16x"
# define PLUGIN_VALUENAME1_164 "TVOC"
# define PLUGIN_VALUENAME2_164 "eCO2"

boolean Plugin_164(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_164;
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
      Device[deviceCount].PluginStats        = true;
      Device[deviceCount].I2CNoDeviceCheck   = true;  //TODO
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_164);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_164));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_164));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { ENS160_I2CADDR_0, ENS160_I2CADDR_1 };
      constexpr int nrAddressOptions   = NR_ELEMENTS(i2cAddressValues);

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), nrAddressOptions, i2cAddressValues, P164_I2C_ADDR);
        addFormNote(F("ADDR Low=0x52, High=0x53"));
      } else {
        success = intArrayContains(nrAddressOptions, i2cAddressValues, event->Par1);
      }

      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P164_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P164_I2C_ADDR = ENS160_I2CADDR_1;
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P164_data_struct(event));
      P164_data_struct *P164_data = static_cast<P164_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P164_data && P164_data->begin());
      break;
    }

    case PLUGIN_READ:
    {
      P164_data_struct *P164_data = static_cast<P164_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P164_data) {
        Serial.print("P164: plugin_read NULLPTR");
        break;
      }

      float temperature = 20.0f;  // A reasonable value in case temperature source task is invalid
      float humidity = 50.0f;     // A reasonable value in case humidity source task is invalid
      if (validTaskIndex(P164_PCONFIG_TEMP_TASK) && validTaskIndex(P164_PCONFIG_HUM_TASK))
      {
        // we're checking a var from another task, so calculate that basevar
        temperature = UserVar[P164_PCONFIG_TEMP_TASK * VARS_PER_TASK + P164_PCONFIG_TEMP_VAL]; // in degrees C
        humidity = UserVar[P164_PCONFIG_HUM_TASK * VARS_PER_TASK + P164_PCONFIG_HUM_VAL];    // in % relative
      }
      success = P164_data->read(UserVar[event->BaseVarIndex], UserVar[event->BaseVarIndex + 1], temperature, humidity);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = P164_data_struct::webformLoad(event);
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      success = P164_data_struct::webformSave(event);
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P164_data_struct *P164_data =
        static_cast<P164_data_struct *>(getPluginTaskData(event->TaskIndex));
      if (nullptr == P164_data) {
        break;
      }
      success = P164_data->tenPerSecond(event);
      success = true;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    case PLUGIN_ONCE_A_SECOND:
    {
      break;
    }
    
  }
  return success;
}

#endif // ifdef USES_P164
