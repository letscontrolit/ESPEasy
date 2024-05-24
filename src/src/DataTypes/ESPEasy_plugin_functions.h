#ifndef DATATYPES_ESPEASY_PLUGIN_DEFS_H
#define DATATYPES_ESPEASY_PLUGIN_DEFS_H

#include "../../ESPEasy_common.h"


// ********************************************************************************
//   Plugin (Task) function calls
// ********************************************************************************
enum PluginFunctions_e {
   PLUGIN_INIT_ALL                    , // Not implemented in a plugin, only called during boot
   PLUGIN_INIT                        , // Init the task, called when task is set to enabled (also at boot)
   PLUGIN_READ                        , // This call can yield new data (when success = true) and then send to controllers
   PLUGIN_ONCE_A_SECOND               , // Called once a second
   PLUGIN_TEN_PER_SECOND              , // Called 10x per second (typical for checking new data instead of waiting)
   PLUGIN_DEVICE_ADD                  , // Called at boot for letting a plugin adding itself to list of available plugins/devices
   PLUGIN_EVENTLIST_ADD               , // Not used.
   PLUGIN_WEBFORM_SAVE                , // Call from web interface to save settings
   PLUGIN_WEBFORM_LOAD                , // Call from web interface for presenting settings and status of plugin
   PLUGIN_WEBFORM_SHOW_VALUES         , // Call from devices overview page to format values in HTML
   PLUGIN_GET_DEVICENAME              , // Call to get the plugin description (e.g. "Switch input - Switch")
   PLUGIN_GET_DEVICEVALUENAMES        , // Call to let the plugin generate some default value names when not defined.
   PLUGIN_GET_DEVICEVALUECOUNT        , // Optional function call to allow tasks to specify the number of output values (e.g. P026_Sysinfo.ino)
   PLUGIN_GET_DEVICEVTYPE             , // Only needed when Device[deviceCount].OutputDataType is not Output_Data_type_t::Default
   PLUGIN_WRITE                       , // Called to allow a task to process a command. Must return success = true when it can handle the command.
// PLUGIN_EVENT_OUT                   , // Does not seem to be used
   PLUGIN_WEBFORM_SHOW_CONFIG         , // Called to show non default pin assignment or addresses like for plugins using serial or 1-Wire
   PLUGIN_SERIAL_IN                   , // Called on received data via serial port Serial0 (N.B. this may conflict with sending commands via serial)
   PLUGIN_UDP_IN                      , // Called for received UDP data via ESPEasy p2p which isn't a standard p2p packet. (See C013 for handling standard p2p packets)
   PLUGIN_CLOCK_IN                    , // Called every new minute
   PLUGIN_TASKTIMER_IN                , // Called with a previously defined event at a specific time, set via setPluginTaskTimer
   PLUGIN_FIFTY_PER_SECOND            , // Called 50 times per second
   PLUGIN_SET_CONFIG                  , // Counterpart of PLUGIN_GET_CONFIG_VALUE to allow to set a config via a command.
   PLUGIN_GET_DEVICEGPIONAMES         , // Allow for specific formatting of the label for standard pin configuration (e.g. "GPIO <- TX")
   PLUGIN_EXIT                        , // Called when a task no longer is enabled (or deleted)
   PLUGIN_GET_CONFIG_VALUE            , // Similar to PLUGIN_WRITE, but meant to fetch some information. Must return success = true when it can handle the command.  Can also be used to access extra unused task values.
//   PLUGIN_UNCONDITIONAL_POLL          , // Used to be called 10x per sec, but no longer used as GPIO related plugins now use a different technique.
   PLUGIN_REQUEST                     , // Specific command to fetch a state (FIXME TD-er: Seems very similar to PLUGIN_GET_CONFIG_VALUE)
   PLUGIN_TIME_CHANGE                 , // Called when system time is set (e.g. via NTP)
   PLUGIN_MONITOR                     , // Replaces PLUGIN_UNCONDITIONAL_POLL
   PLUGIN_SET_DEFAULTS                , // Called when assigning a plugin to a task, to set some default config.
   PLUGIN_GET_PACKED_RAW_DATA         , // Return all data in a compact binary format specific for that plugin.
                                        // Needs FEATURE_PACKED_RAW_DATA
   PLUGIN_DEVICETIMER_IN              , // Similar to PLUGIN_TASKTIMER_IN, addressed to a plugin instead of a task.
   PLUGIN_WEBFORM_SHOW_I2C_PARAMS     , // Show I2C parameters like address.
   PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS  , // When needed, show additional parameters like baudrate or specific serial config
   PLUGIN_MQTT_CONNECTION_STATE       , // Signal when connection to MQTT broker is re-established
   PLUGIN_MQTT_IMPORT                 , // For P037 MQTT import
   PLUGIN_FORMAT_USERVAR              , // Allow plugin specific formatting of a task variable (event->idx = variable)
   PLUGIN_WEBFORM_SHOW_GPIO_DESCR     , // Show GPIO description on devices overview tab
#if FEATURE_PLUGIN_STATS
   PLUGIN_WEBFORM_LOAD_SHOW_STATS     , // Show PluginStats on task config page
#endif // if FEATURE_PLUGIN_STATS
   PLUGIN_I2C_HAS_ADDRESS             , // Check the I2C addresses from the plugin, output in 'success'
   PLUGIN_I2C_GET_ADDRESS             , // Get the current I2C addresses from the plugin, output in 'event->Par1' and 'success'
   PLUGIN_GET_DISPLAY_PARAMETERS      , // Fetch X/Y resolution and Rotation setting from the plugin, output in 'success'
   PLUGIN_WEBFORM_SHOW_ERRORSTATE_OPT , // Show Error State Value options, so be saved during PLUGIN_WEBFORM_SAVE
   PLUGIN_INIT_VALUE_RANGES           , // Initialize the ranges of values, called just before PLUGIN_INIT
   PLUGIN_READ_ERROR_OCCURED          , // Function returns "true" when last measurement was an error, called when PLUGIN_READ returns false
   PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR, // Show the configuration for output type and what value to set to which taskvalue
   PLUGIN_PROCESS_CONTROLLER_DATA     , // Can be called from the controller to signal the plugin to generate (or handle) sending the data.
   PLUGIN_PRIORITY_INIT_ALL           , // Pre-initialize all plugins that are set to PowerManager priority (not implemented in plugins)
   PLUGIN_PRIORITY_INIT               , // Pre-initialize a singe plugins that is set to PowerManager priority
   PLUGIN_WEBFORM_LOAD_ALWAYS         , // Loaded *after* PLUGIN_WEBFORM_LOAD, also shown for remote data-feed devices
#ifdef USES_ESPEASY_NOW
   PLUGIN_FILTEROUT_CONTROLLER_DATA   , // Can be called from the controller to query a task whether the data should be processed further.
#endif
   PLUGIN_WEBFORM_PRE_SERIAL_PARAMS   , // Before serial parameters, convert additional parameters like baudrate or specific serial config

   PLUGIN_MAX_FUNCTION  // Leave as last one.
};


#define NrBitsPluginFunctions   NR_BITS(static_cast<unsigned>(PLUGIN_MAX_FUNCTION))

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
