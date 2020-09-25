/* This file is a template for Plugins */

/* References:
   https://www.letscontrolit.com/wiki/index.php/ESPEasyDevelopment
   https://www.letscontrolit.com/wiki/index.php/ESPEasyDevelopmentGuidelines
   https://github.com/letscontrolit/ESPEasyPluginPlayground
   https://diyprojects.io/esp-easy-develop-plugins/

   A Plugin should have an ID.
   The official plugin list is available here: https://www.letscontrolit.com/wiki/index.php/Official_plugin_list
   The plugin playground is available here: https://github.com/letscontrolit/ESPEasyPluginPlayground

   Use the next available ID. The maximum number of Plugins is defined in ESPEasy-Globals.h (PLUGIN_MAX)

   The Plugin filename should be of the form "_Pxxx_name.ino", where:
    xxx is the ID
    <name> is a short name of the Plugin
   As an example: "_P001_Switch.ino"

   Hints for plugin development:
   - plugins should ideally be added without changes in the framework
   - avoid including libraries. Include the necessary code in the plugin
   - when verifying the plugin check the following:
     - memory used (try different scenarios: plugin enabled, plugin in use, commands executed, plugin disabled, device added/removed)
     - other tests??
   - the development life-cycle is:
     - implement plugin and perform testing
     - set plugin status to DEVELOPMENT and distribute to other users for testing
     - after sufficient usage and possible code correction, set plugin status to TESTING and perform testing with more users
     - finally, plugin will be accepted in project
   - along with the plugin source code, prepare a wiki page containing:
     - instructions on how to make the necessary configuration
     - instructions on commands (if any)
     - examples: plugin usage, command usage,...
   - when a plugin is removed (deleted), make sure you free any memory it uses. Use PLUGIN_EXIT for that
   - if your plugin creates log entries, prefix your entries with your plugin id: "[Pxxx] my plugin did this"
   - if your plugin takes input from user and/or accepts/sends http commands, make sure you properly handle non-alphanumeric characters
      correctly
   - After ESP boots, all devices can send data instantly. If your plugin is for a sensor which sends data, ensure it doesn't need a delay
      before receiving data
   - ensure the plugin does not create sideffects (eg. crashes) if there's no actual device connected
   - check the device's return values. Ensure that if the device returns an invalid value, to use a value like 0 or null to avoid
      side-effects
   - extra hints mentioned in: https://github.com/letscontrolit/ESPEasy/issues/698
 */

// #include section
// include libraries here. For example:
// #include <LiquidCrystal_I2C.h>

/*

#define PLUGIN_xxx
#define PLUGIN_ID_xxx     xxx           // plugin id
#define PLUGIN_NAME_xxx   "Plugin Name" // "Plugin Name" is what will be dislpayed in the selection list
#define PLUGIN_VALUENAME1_xxx "output1" // variable output of the plugin. The label is in quotation marks
#define PLUGIN_VALUENAME2_xxx "output2" // multiple outputs are supported
#define PLUGIN_xxx_DEBUG  false         // set to true for extra log info in the debug


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
//
//   PCONFIG_LABEL(x) is a function to generate a unique label used as HTML id to be able to match 
//                    returned values when saving a configuration.


// Make accessing specific parameters more readable in the code
#define Pxxx_BAUDRATE           PCONFIG_LONG(0)
#define Pxxx_BAUDRATE_LABEL     PCONFIG_LABEL(0)
#define Pxxx_I2C_ADDR           PCONFIG(1)
#define Pxxx_I2C_ADDR_LABEL     PCONFIG_LABEL(1)
#define Pxxx_OUTPUT_TYPE_INDEX  2


// A plugin has to implement the following function

boolean Plugin_xxx(byte function, struct EventStruct *event, String& string)
{
  // function: reason the plugin was called
  // event: ??add description here??
  // string: ??add description here??

  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      // This case defines the device characteristics, edit appropriately

      Device[++deviceCount].Number           = PLUGIN_ID_xxx;                    // Plugin ID number.   (PLUGIN_ID_xxx)
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;               // How the device is connected. e.g. DEVICE_TYPE_SINGLE => connected through 1 datapin
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SWITCH; // Type of value the plugin will return. e.g. SENSOR_TYPE_STRING
      Device[deviceCount].Ports              = 0;                                // Port to use when device has multiple I/O pins  (N.B. not used much)
      Device[deviceCount].ValueCount         = 0;                                // The number of output values of a plugin. The value should match the number of keys PLUGIN_VALUENAME1_xxx
      Device[deviceCount].OutputDataType     = Output_Data_type_t::Default;      // Subset of selectable output data types  (Default = no selection)
      Device[deviceCount].PullUpOption       = false;                            // Allow to set internal pull-up resistors.
      Device[deviceCount].InverseLogicOption = false;                            // Allow to invert the boolean state (e.g. a switch)
      Device[deviceCount].FormulaOption      = false;                            // Allow to enter a formula to convert values during read. (not possible with Custom enabled)
      Device[deviceCount].Custom             = false;
      Device[deviceCount].SendDataOption     = false;                            // Allow to send data to a controller.
      Device[deviceCount].GlobalSyncOption   = true;                             // No longer used. Was used for ESPeasy values sync between nodes
      Device[deviceCount].TimerOption        = false;                            // Allow to set the "Interval" timer for the plugin.
      Device[deviceCount].TimerOptional      = false;                            // When taskdevice timer is not set and not optional, use default "Interval" delay (Settings.Delay)
      Device[deviceCount].DecimalsOnly       = true;                             // Allow to set the number of decimals (otherwise treated a 0 decimals)
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      // return the device name
      string = F(PLUGIN_NAME_xxx);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      // called when the user opens the module configuration page
      // it allows to add a new row for each output variable of the plugin
      // For plugins able to choose output types, see P026_Sysinfo.ino.
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_xxx));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      // Called to show the I2C parameters in the web interface (only called for I2C devices)
      byte choice = Pxxx_I2C_ADDR; // define to get the stored I2C address (e.g. PCONFIG(1))

      int optionValues[16];

      for (byte x = 0; x < 16; x++)
      {
        if (x < 8) {
          optionValues[x] = 0x20 + x;
        }
        else {
          optionValues[x] = 0x30 + x;
        }
      }
      addFormSelectorI2C(Pxxx_I2C_ADDR_LABEL, 16, optionValues, choice);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
      // Called to show optinal extra UART parameters in the web interface (only called for SERIAL devices)
      addFormNumericBox(F("Baudrate"), Pxxx_BAUDRATE_LABEL, Pxxx_BAUDRATE, 2400, 115200);
      addUnit(F("baud"));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      // Called to show non default pin assignment or addresses like for plugins using serial or 1-Wire
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      // This is only called when Device[deviceCount].OutputDataType is not Output_Data_type_t::Default
      // The position in the config parameters used in this example is PCONFIG(Pxxx_OUTPUT_TYPE_INDEX)
      // Must match the one used in case PLUGIN_GET_DEVICEVTYPE  (best to use a define for it)
      // see P026_Sysinfo.ino for more examples.
      event->Par1 = getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(Pxxx_OUTPUT_TYPE_INDEX)));
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      // This is only called when Device[deviceCount].OutputDataType is not Output_Data_type_t::Default
      // The position in the config parameters used in this example is PCONFIG(Pxxx_OUTPUT_TYPE_INDEX)
      // Must match the one used in case PLUGIN_GET_DEVICEVALUECOUNT  (best to use a define for it)
      // IDX is used here to mark the PCONFIG position used to store the Device VType.
      // see P026_Sysinfo.ino for more examples.
      event->idx        = Pxxx_OUTPUT_TYPE_INDEX;
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(event->idx));
      success           = true;
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
      addHtml(F("<TR><TD>Analog Pin:<TD>"));


      // For strings, always use the F() macro, which stores the string in flash, not in memory.

      // String dropdown[5] = { F("option1"), F("option2"), F("option3"), F("option4")};
      // addFormSelector(string, F("drop-down menu"), F("plugin_xxx_displtype"), 4, dropdown, NULL, PCONFIG(0));

      // number selection (min-value - max-value)
      addFormNumericBox(string, F("description"), F("plugin_xxx_description"), PCONFIG(1), min - value, max - value);

      // after the form has been loaded, set success and break
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // this case defines the code to be executed when the form is submitted
      // the plugin settings should be saved to PCONFIG(x)
      // ping configuration should be read from CONFIG_PIN1 and stored

      // after the form has been saved successfuly, set success and break
      success = true;
      break;
    }
    case PLUGIN_INIT:
    {
      // this case defines code to be executed when the plugin is initialised

      // after the plugin has been initialised successfuly, set success and break
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      // code to be executed to read data
      // It is executed according to the delay configured on the device configuration page, only once

      // after the plugin has read data successfuly, set success and break
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      // this case defines code to be executed when the plugin executes an action (command).
      // Commands can be accessed via rules or via http.
      // As an example, http://192.168.1.12//control?cmd=dothis
      // implies that there exists the comamnd "dothis"

      if (plugin_not_initialised) {
        break;
      }

      // FIXME TD-er: This one is not using parseString* function
      // parse string to extract the command
      String tmpString = string;
      int    argIndex  = tmpString.indexOf(',');

      if (argIndex) {
        tmpString = tmpString.substring(0, argIndex);
      }

      String tmpStr = string;
      int    comma1 = tmpStr.indexOf(',');

      if (tmpString.equalsIgnoreCase(F("dothis"))) {
        // do something
        success = true; // set to true only if plugin has executed a command successfully
      }

      break;
    }

    case PLUGIN_EXIT:
    {
      // perform cleanup tasks here. For example, free memory

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

      success = true;
    }
  } // switch
  return success;
}   // function

// implement plugin specific procedures and functions here
void pxxx_do_sth_useful()
{
  // code
}
*/