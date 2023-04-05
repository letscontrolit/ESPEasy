#ifndef DATATYPES_ESPEASY_PLUGIN_DEFS_H
#define DATATYPES_ESPEASY_PLUGIN_DEFS_H

#include "../../ESPEasy_common.h"


// ********************************************************************************
//   Plugin (Task) function calls
// ********************************************************************************
#define PLUGIN_INIT_ALL                     1 // Not implemented in a plugin, only called during boot
#define PLUGIN_INIT                         2 // Init the task, called when task is set to enabled (also at boot)
#define PLUGIN_READ                         3 // This call can yield new data (when success = true) and then send to controllers
#define PLUGIN_ONCE_A_SECOND                4 // Called once a second
#define PLUGIN_TEN_PER_SECOND               5 // Called 10x per second (typical for checking new data instead of waiting)
#define PLUGIN_DEVICE_ADD                   6 // Called at boot for letting a plugin adding itself to list of available plugins/devices
#define PLUGIN_EVENTLIST_ADD                7 // Not used.
#define PLUGIN_WEBFORM_SAVE                 8 // Call from web interface to save settings
#define PLUGIN_WEBFORM_LOAD                 9 // Call from web interface for presenting settings and status of plugin
#define PLUGIN_WEBFORM_SHOW_VALUES         10 // Call from devices overview page to format values in HTML
#define PLUGIN_GET_DEVICENAME              11 // Call to get the plugin description (e.g. "Switch input - Switch")
#define PLUGIN_GET_DEVICEVALUENAMES        12 // Call to let the plugin generate some default value names when not defined.
#define PLUGIN_GET_DEVICEVALUECOUNT        13 // Optional function call to allow tasks to specify the number of output values (e.g. P026_Sysinfo.ino)
#define PLUGIN_GET_DEVICEVTYPE             14 // Only needed when Device[deviceCount].OutputDataType is not Output_Data_type_t::Default
#define PLUGIN_WRITE                       15 // Called to allow a task to process a command. Must return success = true when it can handle the command.
#define PLUGIN_EVENT_OUT                   16 // Does not seem to be used
#define PLUGIN_WEBFORM_SHOW_CONFIG         17 // Called to show non default pin assignment or addresses like for plugins using serial or 1-Wire
#define PLUGIN_SERIAL_IN                   18 // Called on received data via serial port Serial0 (N.B. this may conflict with sending commands via serial)
#define PLUGIN_UDP_IN                      19 // Called for received UDP data via ESPEasy p2p which isn't a standard p2p packet. (See C013 for handling standard p2p packets)
#define PLUGIN_CLOCK_IN                    20 // Called every new minute
#define PLUGIN_TASKTIMER_IN                21 // Called with a previously defined event at a specific time, set via setPluginTaskTimer
#define PLUGIN_FIFTY_PER_SECOND            22 // Called 50 times per second
#define PLUGIN_SET_CONFIG                  23 // Counterpart of PLUGIN_GET_CONFIG_VALUE to allow to set a config via a command.
#define PLUGIN_GET_DEVICEGPIONAMES         24 // Allow for specific formatting of the label for standard pin configuration (e.g. "GPIO <- TX")
#define PLUGIN_EXIT                        25 // Called when a task no longer is enabled (or deleted)
#define PLUGIN_GET_CONFIG_VALUE            26 // Similar to PLUGIN_WRITE, but meant to fetch some information. Must return success = true when it can handle the command.  Can also be used to access extra unused task values.
#define PLUGIN_UNCONDITIONAL_POLL          27 // Used to be called 10x per sec, but no longer used as GPIO related plugins now use a different technique.
#define PLUGIN_REQUEST                     28 // Specific command to fetch a state (FIXME TD-er: Seems very similar to PLUGIN_GET_CONFIG_VALUE)
#define PLUGIN_TIME_CHANGE                 29 // Called when system time is set (e.g. via NTP)
#define PLUGIN_MONITOR                     30 // Replaces PLUGIN_UNCONDITIONAL_POLL
#define PLUGIN_SET_DEFAULTS                31 // Called when assigning a plugin to a task, to set some default config.
#define PLUGIN_GET_PACKED_RAW_DATA         32 // Return all data in a compact binary format specific for that plugin.
                                              // Needs FEATURE_PACKED_RAW_DATA
#define PLUGIN_DEVICETIMER_IN              33 // Similar to PLUGIN_TASKTIMER_IN, addressed to a plugin instead of a task.
#define PLUGIN_WEBFORM_SHOW_I2C_PARAMS     34 // Show I2C parameters like address.
#define PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS  35 // When needed, show additional parameters like baudrate or specific serial config
#define PLUGIN_MQTT_CONNECTION_STATE       36 // Signal when connection to MQTT broker is re-established
#define PLUGIN_MQTT_IMPORT                 37 // For P037 MQTT import
#define PLUGIN_FORMAT_USERVAR              38 // Allow plugin specific formatting of a task variable (event->idx = variable)
#define PLUGIN_WEBFORM_SHOW_GPIO_DESCR     39 // Show GPIO description on devices overview tab
#if FEATURE_PLUGIN_STATS
#define PLUGIN_WEBFORM_LOAD_SHOW_STATS     40 // Show PluginStats on task config page
#endif // if FEATURE_PLUGIN_STATS
#define PLUGIN_I2C_HAS_ADDRESS             41 // Check the I2C addresses from the plugin, output in 'success'
#define PLUGIN_I2C_GET_ADDRESS             42 // Get the current I2C addresses from the plugin, output in 'event->Par1' and 'success'
#define PLUGIN_GET_DISPLAY_PARAMETERS      43 // Fetch X/Y resolution and Rotation setting from the plugin, output in 'success'
#define PLUGIN_WEBFORM_SHOW_ERRORSTATE_OPT 44 // Show Error State Value options, so be saved during PLUGIN_WEBFORM_SAVE
#define PLUGIN_INIT_VALUE_RANGES           45 // Initialize the ranges of values, called just before PLUGIN_INIT
#define PLUGIN_READ_ERROR_OCCURED          46 // Function returns "true" when last measurement was an error, called when PLUGIN_READ returns false
#define PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR 47 // Show the configuration for output type and what value to set to which taskvalue
#define PLUGIN_PROCESS_CONTROLLER_DATA     48 // Can be called from the controller to signal the plugin to generate (or handle) sending the data.
#define PLUGIN_PRIORITY_INIT_ALL           49 // Pre-initialize all plugins that are set to PowerManager priority (not implemented in plugins)
#define PLUGIN_PRIORITY_INIT               50 // Pre-initialize a singe plugins that is set to PowerManager priority




// ********************************************************************************
//   CPlugin (Controller) function calls
// ********************************************************************************

class CPlugin {
public:

  // As these function values are also used in the timing stats, make sure there is no overlap with the PLUGIN_xxx numbering.

  enum class Function {
    CPLUGIN_PROTOCOL_ADD = 127, // Called at boot for letting a controller adding itself to list of available controllers
    CPLUGIN_PROTOCOL_TEMPLATE,
    CPLUGIN_PROTOCOL_SEND,
    CPLUGIN_PROTOCOL_RECV,
    CPLUGIN_GET_DEVICENAME,
    CPLUGIN_WEBFORM_SAVE,
    CPLUGIN_WEBFORM_LOAD,
    CPLUGIN_GET_PROTOCOL_DISPLAY_NAME,
    CPLUGIN_TASK_CHANGE_NOTIFICATION,
    CPLUGIN_INIT,
    CPLUGIN_UDP_IN,
    CPLUGIN_FLUSH,            // Force offloading data stored in buffers, called before sleep/reboot
    CPLUGIN_TEN_PER_SECOND,   // Called 10x per second (typical for checking new data instead of waiting)
    CPLUGIN_FIFTY_PER_SECOND, // Called 50x per second (typical for checking new data instead of waiting)
    CPLUGIN_INIT_ALL,
    CPLUGIN_EXIT,
    CPLUGIN_WRITE,            // Send commands to a controller.


    // new messages for autodiscover controller plugins (experimental) i.e. C014
    CPLUGIN_GOT_CONNECTED,           // call after connected to mqtt server to publich device autodicover features
    CPLUGIN_GOT_INVALID,             // should be called before major changes i.e. changing the device name to clean up data on the
                                     // controller. !ToDo
    CPLUGIN_INTERVAL,                // call every interval loop
    CPLUGIN_ACKNOWLEDGE,             // call for sending acknowledges !ToDo done by direct function call in PluginCall() for now.

    CPLUGIN_WEBFORM_SHOW_HOST_CONFIG // Used for showing host information for the controller.
  };
};

// ********************************************************************************
//   NPlugin (Notification) function calls
// ********************************************************************************
class NPlugin {
public:

  enum class Function {
    NPLUGIN_PROTOCOL_ADD = 1,
    NPLUGIN_GET_DEVICENAME,
    NPLUGIN_WEBFORM_SAVE,
    NPLUGIN_WEBFORM_LOAD,
    NPLUGIN_WRITE,
    NPLUGIN_NOTIFY
  };
};


#endif // DATATYPES_ESPEASY_PLUGIN_DEFS_H
