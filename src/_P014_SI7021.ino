#include "_Plugin_Helper.h"
#ifdef USES_P014

// #######################################################################################################
// ######################## Plugin 014 SI7021 I2C Temperature Humidity Sensor  ###########################
// #######################################################################################################
// 12-10-2015 Charles-Henri Hallard, see my projects and blog at https://hallard.me

# include "src/PluginStructs/P014_data_struct.h"

# define PLUGIN_014
# define PLUGIN_ID_014        14
# define PLUGIN_NAME_014       "Environment - SI7021/HTU21D"
# define PLUGIN_VALUENAME1_014 "Temperature"
# define PLUGIN_VALUENAME2_014 "Humidity"


boolean Plugin_014(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_014;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
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
      string = F(PLUGIN_NAME_014);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_014));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_014));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = (event->Par1 == 0x40);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
        # define SI7021_RESOLUTION_OPTION 4

      uint8_t choice = PCONFIG(0);
      const __FlashStringHelper *options[SI7021_RESOLUTION_OPTION];
      int optionValues[SI7021_RESOLUTION_OPTION];
      optionValues[0] = SI7021_RESOLUTION_14T_12RH;
      options[0]      = F("Temp 14 bits / RH 12 bits");
      optionValues[1] = SI7021_RESOLUTION_13T_10RH;
      options[1]      = F("Temp 13 bits / RH 10 bits");
      optionValues[2] = SI7021_RESOLUTION_12T_08RH;
      options[2]      = F("Temp 12 bits / RH  8 bits");
      optionValues[3] = SI7021_RESOLUTION_11T_11RH;
      options[3]      = F("Temp 11 bits / RH 11 bits");
      addFormSelector(F("Resolution"), F("p014_res"), SI7021_RESOLUTION_OPTION, options, optionValues, choice);

      // addUnit(F("bits"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("p014_res"));

      success = true;
      break;
    }


    case PLUGIN_INIT:
    {
      // Get sensor resolution configuration
      uint8_t res = PCONFIG(0);
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P014_data_struct(res));
      P014_data_struct *P014_data =
        static_cast<P014_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P014_data) {
        return success;
      }

      if (P014_data->init()) {
        success = true;
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P014_data_struct *P014_data =
        static_cast<P014_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P014_data)) {
        if (P014_data->loop()) {
          // Update was succesfull, schedule a read.
          Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
        }
      }

      break;
    }

    case PLUGIN_READ:
    {
      P014_data_struct *P014_data =
        static_cast<P014_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P014_data)) {
        success = P014_data->getReadValue(
          UserVar[event->BaseVarIndex],      // temperature
          UserVar[event->BaseVarIndex + 1]); // humidity
      }
      break;
    }
  }
  return success;
}

#endif // USES_P014
