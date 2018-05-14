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
 - if your plugin takes input from user and/or accepts/sends http commands, make sure you properly handle non-alphanumeric characters correctly
 - After ESP boots, all devices can send data instantly. If your plugin is for a sensor which sends data, ensure it doesn't need a delay before receiving data
 - ensure the plugin does not create sideffects (eg. crashes) if there's no actual device connected
- check the device's return values. Ensure that if the device returns an invalid value, to use a value like 0 or null to avoid side-effects
- extra hints mentioned in: https://github.com/letscontrolit/ESPEasy/issues/698
*/
//#include section
//include libraries here. For example:
//#include <LiquidCrystal_I2C.h>

//uncomment one of the following as needed
//#ifdef PLUGIN_BUILD_DEVELOPMENT
//#ifdef PLUGIN_BUILD_TESTING

//#define PLUGIN_xxx
//#define PLUGIN_ID_xxx     xxx               //plugin id
//#define PLUGIN_NAME_xxx   "Plugin Name"     //"Plugin Name" is what will be dislpayed in the selection list
//#define PLUGIN_VALUENAME1_xxx "output1"     //variable output of the plugin. The label is in quotation marks
//#define PLUGIN_VALUENAME2_xxx "output2"     //multiple outputs are supported
//#define PLUGIN_xxx_DEBUG  false             //set to true for extra log info in the debug

/*
PIN/port configuration is stored in the following:
Settings.TaskDevicePin1[event->TaskIndex] - The first GPIO pin selected within the task
Settings.TaskDevicePin2[event->TaskIndex] - The second GPIO pin selected within the task
Settings.TaskDevicePin3[event->TaskIndex] - The third GPIO pin selected within the task
Settings.TaskDevicePort[event->TaskIndex] - The port in case the device has multiple in/out pins

Custom configuration is stored in the following:
Settings.TaskDevicePluginConfig[event->TaskIndex][x]
x can be between 1 - 8 and can store values between -32767 - 32768 (16 bit)

*/

//A plugin has to implement the following function
/*
boolean Plugin_xxx(byte function, struct EventStruct *event, String& string)
{
  //function: reason the plugin was called
  //event: ??add description here??
  // string: ??add description here??

  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
        //This case defines the device characteristics, edit appropriately

        Device[++deviceCount].Number = PLUGIN_ID_xxx;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;  //how the device is connected
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH; //type of value the plugin will return, used only for Domoticz
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;             //number of output variables. The value should match the number of keys PLUGIN_VALUENAME1_xxx
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].TimerOptional = false;
        Device[deviceCount].GlobalSyncOption = true;
        Device[deviceCount].DecimalsOnly = true;
        break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      //return the device name
      string = F(PLUGIN_NAME_xxx);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      //called when the user opens the module configuration page
      //it allows to add a new row for each output variable of the plugin
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_xxx));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      //this case defines what should be displayed on the web form, when this plugin is selected
      //The user's selection will be stored in
      //Settings.TaskDevicePluginConfig[event->TaskIndex][x] (custom configuration)

      // Make sure not to append data to the string variable in this PLUGIN_WEBFORM_LOAD call.
      // This has changed, so now use the appropriate functions to write directly to the Streaming
      // WebServer. This takes much less memory and is faster.
      // There will be an error in the web interface if something is added to the "string" variable.

      //Use any of the following (defined at WebServer.ino):
      //addFormNote(F("not editable text added here"));
      To add some html, which cannot be done in the existing functions, add it in the following way:
      addHtml(F("<TR><TD>Analog Pin:<TD>"));


      For strings, always use the F() macro, which stores the string in flash, not in memory.

      //String dropdown[5] = { F("option1"), F("option2"), F("option3"), F("option4")};
      //addFormSelector(string, F("drop-down menu"), F("plugin_xxx_displtype"), 4, dropdown, NULL, Settings.TaskDevicePluginConfig[event->TaskIndex][0]);

      //number selection (min-value - max-value)
      addFormNumericBox(string, F("description"), F("plugin_xxx_description"), Settings.TaskDevicePluginConfig[event->TaskIndex][1], min-value, max-value);

      //after the form has been loaded, set success and break
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      //this case defines the code to be executed when the form is submitted
      //the plugin settings should be saved to Settings.TaskDevicePluginConfig[event->TaskIndex][x]
      //ping configuration should be read from Settings.TaskDevicePin1[event->TaskIndex] and stored

      //after the form has been saved successfuly, set success and break
      success = true;
      break;

    }
    case PLUGIN_INIT:
    {
      //this case defines code to be executed when the plugin is initialised

      //after the plugin has been initialised successfuly, set success and break
      success = true;
      break;

    }

    case PLUGIN_READ:
    {
      //code to be executed to read data
      //It is executed according to the delay configured on the device configuration page, only once

      //after the plugin has read data successfuly, set success and break
      success = true;
      break;

    }

    case PLUGIN_WRITE:
    {
      //this case defines code to be executed when the plugin executes an action (command).
      //Commands can be accessed via rules or via http.
      //As an example, http://192.168.1.12//control?cmd=dothis
      //implies that there exists the comamnd "dothis"

      if (plugin_not_initialised)
        break;

      //parse string to extract the command
      String tmpString  = string;
      int argIndex = tmpString.indexOf(',');
      if (argIndex)
        tmpString = tmpString.substring(0, argIndex);

      String tmpStr = string;
      int comma1 = tmpStr.indexOf(',');
      if (tmpString.equalsIgnoreCase(F("dothis"))) {
        //do something
        success = true;     //set to true only if plugin has executed a command successfully
      }

       break;
    }

	case PLUGIN_EXIT:
	{
	  //perform cleanup tasks here. For example, free memory

	  break;

	}

    case PLUGIN_ONCE_A_SECOND:
    {
      //code to be executed once a second. Tasks which do not require fast response can be added here

      success = true;

    }

    case PLUGIN_TEN_PER_SECOND:
    {
      //code to be executed 10 times per second. Tasks which require fast response can be added here
      //be careful on what is added here. Heavy processing will result in slowing the module down!

      success = true;

    }
  }   // switch
  return success;

}     //function

//implement plugin specific procedures and functions here
void pxxx_do_sth_useful()
{
  //code

}

*/
