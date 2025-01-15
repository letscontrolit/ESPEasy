// #######################################################################################################
// ################################## Plugin 144: Dust Sensor Ikea Vindriktning ##########################
// #######################################################################################################

/*
   Plugin is based upon various open sources on the internet. Plugin uses the serial lib to access a serial port
   This plugin was written by flashmark

   This plugin reads the particle concentration from the PM1006 sensor in the Ikea Vindriktning
   The original Ikea processor is controlling the PM1006 sensor, the plugin is eavesdropping the responses
   DevicePin1 - RX on ESP, TX on PM1006
   DevicePin2 - TX on ESP, RX on PM1006 (optional, currently not accessed by the plugin)
   This plugin is intended to be extended with stand alone support for the PM1006 and PM1006K sensors
   The stand alone support is not implemented yet
 */

// #include section
#include "_Plugin_Helper.h"
#ifdef USES_P144
# include "src/PluginStructs/P144_data_struct.h" // Sensor abstraction for P144

// Standard plugin defines
# define PLUGIN_144
# define PLUGIN_ID_144     144                               // plugin id
# define PLUGIN_NAME_144   "Dust - PM1006(K) (Vindriktning)" // "Plugin Name" is what will be dislpayed in the selection list
# define PLUGIN_VALUENAME1_144 "PM2.5"                       // variable output of the plugin. The label is in quotation marks

//   PIN/port configuration is stored in the following:
//   CONFIG_PIN1 - Used by plugin_Helper_serial (RX pin)
//   CONFIG_PIN2 - Used by plugin_Helper_serial (TX pin)
//   CONFIG_PIN3 - Not used
//   CONFIG_PORT - Used by the plugin_Helper_serial (serialType)
//
//   Custom configuration is stored in the following:
//   PCONFIG(x)
//   x can be between 1 - 8 and can store values between -32767 - 32768 (16 bit)
//
//   N.B. these are aliases for a longer less readable amount of code. See _Plugin_Helper.h
//
//
//   PCONFIG_LABEL(x) is a function to generate a unique label used as HTML id to be able to match
//                    returned values when saving a configuration.


// A plugin has to implement the following function
boolean Plugin_144(uint8_t function, struct EventStruct *event, String& string)
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

      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_144;
      dev.Type           = DEVICE_TYPE_SERIAL;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.ValueCount     = 1;
      dev.OutputDataType = Output_Data_type_t::Default;
      dev.FormulaOption  = true;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      // return the device name
      string = F(PLUGIN_NAME_144);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      // called when the user opens the module configuration page
      // it allows to add a new row for each output variable of the plugin
      // For plugins able to choose output types, see P026_Sysinfo.ino.
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_144));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_getGpioNames(event, false, true); // TX optional
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      // Called to show non default pin assignment or addresses like for plugins using serial or 1-Wire
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      // Set a default config here, which will be called when a plugin is assigned to a task.
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // this case defines what should be displayed on the web form, when this plugin is selected
      // The user's selection will be stored in
      // PCONFIG(x) (custom configuration)

      // Make sure not to append data to the string variable in this PLUGIN_WEBFORM_LOAD call.
      // This has changed, so now use the appropriate functions to write directly to the Streaming
      // web_server. This takes much less memory and is faster.
      // There will be an error in the web interface if something is added to the "string" variable.

      // Use any of the following (defined at web_server.ino):
      // addFormNote(F("not editable text added here"));
      // To add some html, which cannot be done in the existing functions, add it in the following way:
      /// addRowLabel(F("Analog Pin"));

      // For strings, always use the F() macro, which stores the string in flash, not in memory.

      // String dropdown[5] = { F("option1"), F("option2"), F("option3"), F("option4")};
      // addFormSelector(string, F("drop-down menu"), F("plugin_xxx_displtype"), 4, dropdown, nullptr, PCONFIG(0));

      // number selection (min-value - max-value)
      /// addFormNumericBox(string, F("description"), F("plugin_144_description"), PCONFIG(1), min - value, max - value);

      // after the form has been loaded, set success and break
      // serialHelper_serialconfig_webformLoad(event, true);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // this case defines the code to be executed when the form is submitted
      // the plugin settings should be saved to PCONFIG(x)
      // ping configuration should be read from CONFIG_PIN1 and stored
      // serialHelper_webformSave(event);

      // after the form has been saved successfuly, set success and break
      success = true;
      break;
    }
    case PLUGIN_INIT:
    {
      // this case defines code to be executed when the plugin is initialised
      const int8_t rxPin               = serialHelper_getRxPin(event);
      const int8_t txPin               = serialHelper_getTxPin(event);
      const ESPEasySerialPort portType = serialHelper_getSerialType(event);

      // Create the P144_data_struct object that will do all the sensor interaction
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P144_data_struct());
      P144_data_struct *P144_data =
        static_cast<P144_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P144_data != nullptr)
      {
        success = P144_data->setSerial(portType, rxPin, txPin); // Initialize with existing task data
      }
      break;
    }

    case PLUGIN_READ:
    {
      // code to be executed to read data
      // It is executed according to the delay configured on the device configuration page, only once
      P144_data_struct *P144_data =
        static_cast<P144_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P144_data != nullptr) {
        UserVar.setFloat(event->TaskIndex, 0, P144_data->getValue());
        # ifdef PLUGIN_144_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO))
        {
          addLogMove(LOG_LEVEL_INFO, concat(F("P144 : READ "), UserVar[event->BaseVarIndex]));
        }
        # endif // ifdef PLUGIN_144_DEBUG
      }
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      // this case defines code to be executed when the plugin executes an action (command).
      // Commands can be accessed via rules or via http.
      // As an example, http://192.168.1.12//control?cmd=dothis
      // implies that there exists the comamnd "dothis"

      break;
    }

    case PLUGIN_EXIT:
    {
      // perform cleanup tasks here. For example, free memory

      P144_data_struct *P144_data =
        static_cast<P144_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P144_data != nullptr) {
        P144_data->disconnectSerial();
      }
      success = true;
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      // code to be executed once a second. Tasks which do not require fast response can be added here
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      // code to be executed 10 times per second. Tasks which require fast response can be added here
      // be careful on what is added here. Heavy processing will result in slowing the module down!
      P144_data_struct *P144_data =
        static_cast<P144_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P144_data != nullptr)
      {
        success = P144_data->processSensor();
      }
      break;
    }
  } // switch
  return success;
}   // function

#endif // ifdef USES_P144
