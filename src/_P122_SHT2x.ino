#include "_Plugin_Helper.h"
#ifdef USES_P122

// #######################################################################################################
// ######################## Plugin 122 SHT2x I2C Temperature Humidity Sensor  ############################
// #######################################################################################################
// 26-03-2023 Flashmark creation based upon https://github.com/RobTillaart/SHT2x

# include "src/PluginStructs/P122_data_struct.h"

# define PLUGIN_122
# define PLUGIN_ID_122     122                   // plugin id
# define PLUGIN_NAME_122   "Environment - SHT2x" // What will be dislpayed in the selection list
# define PLUGIN_VALUENAME1_122 "Temperature"     // variable output of the plugin. The label is in quotation marks
# define PLUGIN_VALUENAME2_122 "Humidity"        // multiple outputs are supported

//   PIN/port configuration is stored in the following:
//   CONFIG_PIN1 - The first GPIO pin selected within the task
//   CONFIG_PIN2 - The second GPIO pin selected within the task
//   CONFIG_PIN3 - The third GPIO pin selected within the task
//   CONFIG_PORT - The port in case the device has multiple in/out pins
//
//   Custom configuration is stored in the following:
//   PCONFIG(x)
//   x can be between 1 - 8 and can store values between -32767 - 32768 (16 bit)
//
//   N.B. these are aliases for a longer less readable amount of code. See _Plugin_Helper.h
//
//   PCONFIG_LABEL(x) is a function to generate a unique label used as HTML id to be able to match
//                    returned values when saving a configuration.

// Make accessing specific parameters more readable in the code
// #define Pxxx_OUTPUT_TYPE_INDEX  2
# define P122_I2C_ADDRESS         PCONFIG(0)
# define P122_I2C_ADDRESS_LABEL   PCONFIG_LABEL(0)
# define P122_RESOLUTION          PCONFIG(1)
# define P122_RESOLUTION_LABEL    PCONFIG_LABEL(1)

// A plugin has to implement the following function

boolean Plugin_122(uint8_t function, struct EventStruct *event, String& string)
{
  // function: reason the plugin was called
  // event: ??add description here??
  // string: ??add description here??

  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      // This case defines the device characteristics
      Device[++deviceCount].Number           = PLUGIN_ID_122;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].I2CNoDeviceCheck   = true;

      // Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats    = true;
      Device[deviceCount].OutputDataType = Output_Data_type_t::Default;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      // return the device name
      string = F(PLUGIN_NAME_122);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      // called when the user opens the module configuration page
      // it allows to add a new row for each output variable of the plugin
      // For plugins able to choose output types, see P026_Sysinfo.ino.
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_122));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_122));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      // Set a default config here, which will be called when a plugin is assigned to a task.
      P122_I2C_ADDRESS = P122_I2C_ADDRESS_AD0_0;
      P122_RESOLUTION  = P122_RESOLUTION_14T_12RH;
      success          = true;
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P122_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { P122_I2C_ADDRESS_AD0_0, P122_I2C_ADDRESS_AD0_1 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS)
      {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, P122_I2C_ADDRESS);
        addFormNote(F("ADO Low=0x40, High=0x41"));
      }
      else
      {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // this case defines what should be displayed on the web form, when this plugin is selected
      // The user's selection will be stored in
      // PCONFIG(x) (custom configuration)

      # define P122_RESOLUTION_OPTIONS 4

      const __FlashStringHelper *options[] = {
        F("Temp 14 bits / RH 12 bits"),
        F("Temp 13 bits / RH 10 bits"),
        F("Temp 12 bits / RH  8 bits"),
        F("Temp 11 bits / RH 11 bits"),
      };
      const int optionValues[] = {
        P122_RESOLUTION_14T_12RH,
        P122_RESOLUTION_13T_10RH,
        P122_RESOLUTION_12T_08RH,
        P122_RESOLUTION_11T_11RH,
      };
      addFormSelector(F("Resolution"), P122_RESOLUTION_LABEL, P122_RESOLUTION_OPTIONS, options, optionValues, P122_RESOLUTION);

# ifndef LIMIT_BUILD_SIZE
      P122_data_struct *P122_data = static_cast<P122_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P122_data != nullptr)
      {
        uint32_t eida;
        uint32_t eidb;
        uint8_t  firmware;
        P122_data->getEID(eida, eidb, firmware);
        String txt = F("CHIP ID:");
        txt += formatToHex(eida);
        txt += ',';
        txt += formatToHex(eidb);
        txt += F(" firmware=");
        txt += String(firmware);
#  ifdef PLUGIN_122_DEBUG
        txt += F(" userReg= ");
        txt += formatToHex(P122_data->getUserReg());
#  endif // ifdef PLUGIN_122_DEBUG
        addFormNote(txt);
      }
# endif // ifndef LIMIT_BUILD_SIZE
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // this case defines the code to be executed when the form is submitted
      // the plugin settings should be saved to PCONFIG(x)
      // ping configuration should be read from CONFIG_PIN1 and stored

      // after the form has been saved successfuly, set success and break
      P122_I2C_ADDRESS = getFormItemInt(F("i2c_addr"));
      P122_RESOLUTION  = getFormItemInt(P122_RESOLUTION_LABEL);
      success          = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // this case defines code to be executed when the plugin is initialised
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P122_data_struct());
      P122_data_struct *P122_data = static_cast<P122_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P122_data != nullptr)
      {
        P122_data->setupDevice(P122_I2C_ADDRESS, P122_RESOLUTION);
        P122_data->reset();
        success = true;
      }
      UserVar[event->BaseVarIndex]     = NAN;
      UserVar[event->BaseVarIndex + 1] = NAN;
      UserVar[event->BaseVarIndex + 2] = NAN;
      break;
    }

    case PLUGIN_READ:
    {
      // code to be executed to read data
      // It is executed according to the delay configured on the device configuration page, only once
      P122_data_struct *P122_data = static_cast<P122_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P122_data)
      {
        if (P122_data->inError())
        {
          UserVar[event->BaseVarIndex]     = NAN;
          UserVar[event->BaseVarIndex + 1] = NAN;
          addLog(LOG_LEVEL_ERROR, F("SHT2x: in Error!"));
        }
        else if (P122_data->newValues())
        {
          UserVar[event->BaseVarIndex]     = P122_data->getTemperature();
          UserVar[event->BaseVarIndex + 1] = P122_data->getHumidity();
          P122_data->startMeasurements(); // getting ready for another read cycle
        }
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO))
      {
        String log = F("P122: Temperature: ");
        log += UserVar[event->BaseVarIndex + 0];
        log += F(" Humidity: ");
        log += UserVar[event->BaseVarIndex + 1];
        addLog(LOG_LEVEL_INFO, log);
      }
      success = true;
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      // code to be executed once a second. Tasks which do not require fast response can be added here
      success = true;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      // code to be executed 10 times per second. Tasks which require fast response can be added here
      // be careful on what is added here. Heavy processing will result in slowing the module down!
      P122_data_struct *P122_data = static_cast<P122_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P122_data)
      {
        P122_data->update(); // SHT2x FSM evaluation
      }
      success = true;
    }
  } // switch
  return success;
}   // function

#endif  //USES_P122
