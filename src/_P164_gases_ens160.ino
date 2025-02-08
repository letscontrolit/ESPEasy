#include "_Plugin_Helper.h"
#ifdef USES_P164

// #######################################################################################################
// #################################### Plugin-164: Gases - ENS16x (tvoc,eco2) ###########################
// #######################################################################################################
// P164 "GASES - ENS16x (TVOC, eCO2)"
// Plugin for ENS160 & ENS161 TVOC and eCO2 sensor with I2C interface from ScioSense
// For documentation of the ENS160 hardware device see
// https://www.sciosense.com/wp-content/uploads/documents/SC-001224-DS-9-ENS160-Datasheet.pdf
//
// PLugin code:
// 2023 By flashmark
// #######################################################################################################

# include "src/PluginStructs/P164_data_struct.h"

# define PLUGIN_164
# define PLUGIN_ID_164         164
# define PLUGIN_NAME_164       "Gases - ENS16x"
# define PLUGIN_VALUENAME1_164 "TVOC"
# define PLUGIN_VALUENAME2_164 "eCO2"
# define PLUGIN_VALUENAME3_164 "AQI"

boolean Plugin_164(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_164;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_DUAL;
      dev.FormulaOption  = true;
      dev.ValueCount     = 3;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
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
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_164));

      for (int i = 0; i < 3; ++i) {
        ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
      }
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { P164_ENS160_I2CADDR_0, P164_ENS160_I2CADDR_1 };
      constexpr int nrAddressOptions   = NR_ELEMENTS(i2cAddressValues);

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), nrAddressOptions, i2cAddressValues, P164_PCONFIG_I2C_ADDR);
        addFormNote(F("ADDR Low=0x52, High=0x53"));
      } else {
        success = intArrayContains(nrAddressOptions, i2cAddressValues, event->Par1);
      }

      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P164_PCONFIG_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P164_PCONFIG_I2C_ADDR = P164_ENS160_I2CADDR_1;
      success               = true;
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
        addLogMove(LOG_LEVEL_ERROR, F("P164: plugin_read NULLPTR"));
        break;
      }

      float temperature = 20.0f; // A reasonable value in case temperature source task is invalid
      float humidity    = 50.0f; // A reasonable value in case humidity source task is invalid
      float tvoc        = 0.0f;  // tvoc value to be retrieved from device
      float eco2        = 0.0f;  // eCO2 value to be retrieved from device
      float aqi         = 0.0f;  // AQI value to be retrieved from device

      if (validTaskIndex(P164_PCONFIG_TEMP_TASK) && validTaskIndex(P164_PCONFIG_HUM_TASK))
      {
        // we're checking a value from other tasks
        temperature = UserVar.getFloat(P164_PCONFIG_TEMP_TASK, P164_PCONFIG_TEMP_VAL); // in degrees C
        humidity    = UserVar.getFloat(P164_PCONFIG_HUM_TASK, P164_PCONFIG_HUM_VAL);   // in % relative
      }
      success = P164_data->read(tvoc, eco2, aqi, temperature, humidity);
      UserVar.setFloat(event->TaskIndex, 0, tvoc);
      UserVar.setFloat(event->TaskIndex, 1, eco2);
      UserVar.setFloat(event->TaskIndex, 2, aqi);
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
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P164
