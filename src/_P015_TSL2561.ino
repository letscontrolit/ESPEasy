#include "_Plugin_Helper.h"

#ifdef USES_P015

// #######################################################################################################
// ######################## Plugin 015 TSL2561 I2C Lux Sensor ############################################
// #######################################################################################################
// complete rewrite, to support lower lux values better, add ability to change gain and sleep mode
// by: https://github.com/krikk
// this plugin is based on the sparkfun library
// written based on version 1.1.0 from https://github.com/sparkfun/SparkFun_TSL2561_Arduino_Library


#include "src/PluginStructs/P015_data_struct.h"

#define PLUGIN_015
#define PLUGIN_ID_015        15
#define PLUGIN_NAME_015       "Light/Lux - TSL2561"
#define PLUGIN_VALUENAME1_015 "Lux"
#define PLUGIN_VALUENAME2_015 "Infrared"
#define PLUGIN_VALUENAME3_015 "Broadband"
#define PLUGIN_VALUENAME4_015 "Ratio"


#define P015_I2C_ADDR    PCONFIG(0)
#define P015_INTEGRATION PCONFIG(1)
#define P015_SLEEP       PCONFIG(2)
#define P015_GAIN        PCONFIG(3)


boolean Plugin_015(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_015;
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
      string = F(PLUGIN_NAME_015);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_015));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_015));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_015));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_015));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      /*
          String options1[3];
          options1[0] = F("0x39 - (default)");
          options1[1] = F("0x49");
          options1[2] = F("0x29");
       */
      int optionValues[3];
      optionValues[0] = TSL2561_ADDR;
      optionValues[1] = TSL2561_ADDR_1;
      optionValues[2] = TSL2561_ADDR_0;
      addFormSelectorI2C(F("p015_tsl2561_i2c"), 3, optionValues, P015_I2C_ADDR);
      break;
    }


    case PLUGIN_WEBFORM_LOAD:
    {
      {
        #define TSL2561_INTEGRATION_OPTION 3
        String options[TSL2561_INTEGRATION_OPTION];
        int    optionValues[TSL2561_INTEGRATION_OPTION];
        optionValues[0] = 0x00;
        options[0]      = F("13.7 ms");
        optionValues[1] = 0x01;
        options[1]      = F("101 ms");
        optionValues[2] = 0x02;
        options[2]      = F("402 ms");
        addFormSelector(F("Integration time"), F("p015_integration"), TSL2561_INTEGRATION_OPTION, options, optionValues, P015_INTEGRATION);
      }

      addFormCheckBox(F("Send sensor to sleep:"), F("p015_sleep"),
                      P015_SLEEP);

      {
        #define TSL2561_GAIN_OPTION 4
        String options[TSL2561_GAIN_OPTION];
        int    optionValues[TSL2561_GAIN_OPTION];
        optionValues[0] = P015_NO_GAIN;
        options[0]      = F("No Gain");
        optionValues[1] = P015_16X_GAIN;
        options[1]      = F("16x Gain");
        optionValues[2] = P015_AUTO_GAIN;
        options[2]      = F("Auto Gain");
        optionValues[3] = P015_EXT_AUTO_GAIN;
        options[3]      = F("Extended Auto Gain");
        addFormSelector(F("Gain"), F("p015_gain"), TSL2561_GAIN_OPTION, options, optionValues, P015_GAIN);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P015_I2C_ADDR    = getFormItemInt(F("p015_tsl2561_i2c"));
      P015_INTEGRATION = getFormItemInt(F("p015_integration"));
      P015_SLEEP       = isFormItemChecked(F("p015_sleep"));
      P015_GAIN        = getFormItemInt(F("p015_gain"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P015_data_struct(P015_I2C_ADDR, P015_GAIN, P015_INTEGRATION));
      P015_data_struct *P015_data =
        static_cast<P015_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P015_data) {
        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {
      P015_data_struct *P015_data =
        static_cast<P015_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P015_data) {
        P015_data->begin();

        success = P015_data->performRead(
          UserVar[event->BaseVarIndex],      // lux
          UserVar[event->BaseVarIndex + 1],  // infrared
          UserVar[event->BaseVarIndex + 2],  // broadband
          UserVar[event->BaseVarIndex + 3]); // ir_broadband_ratio

        if (P015_SLEEP) {
          addLog(LOG_LEVEL_DEBUG_MORE, F("TSL2561: sleeping..."));
          P015_data->setPowerDown();
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P015
