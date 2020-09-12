#ifndef ESPEASY_PLUGIN_DEFS_H
#define ESPEASY_PLUGIN_DEFS_H


// ********************************************************************************
//   Plugin (Task) function calls
// ********************************************************************************
#define PLUGIN_INIT_ALL                     1
#define PLUGIN_INIT                         2
#define PLUGIN_READ                         3 // This call can yield new data (when success = true) and then send to controllers
#define PLUGIN_ONCE_A_SECOND                4 // Called once a second
#define PLUGIN_TEN_PER_SECOND               5 // Called 10x per second (typical for checking new data instead of waiting)
#define PLUGIN_DEVICE_ADD                   6 // Called at boot for letting a plugin adding itself to list of available plugins/devices
#define PLUGIN_EVENTLIST_ADD                7
#define PLUGIN_WEBFORM_SAVE                 8 // Call from web interface to save settings
#define PLUGIN_WEBFORM_LOAD                 9 // Call from web interface for presenting settings and status of plugin
#define PLUGIN_WEBFORM_SHOW_VALUES         10 // Call from devices overview page to format values in HTML
#define PLUGIN_GET_DEVICENAME              11
#define PLUGIN_GET_DEVICEVALUENAMES        12
#define PLUGIN_WRITE                       13
#define PLUGIN_EVENT_OUT                   14
#define PLUGIN_WEBFORM_SHOW_CONFIG         15
#define PLUGIN_SERIAL_IN                   16
#define PLUGIN_UDP_IN                      17
#define PLUGIN_CLOCK_IN                    18
#define PLUGIN_TIMER_IN                    19
#define PLUGIN_FIFTY_PER_SECOND            20
#define PLUGIN_SET_CONFIG                  21
#define PLUGIN_GET_DEVICEGPIONAMES         22
#define PLUGIN_EXIT                        23
#define PLUGIN_GET_CONFIG                  24
#define PLUGIN_UNCONDITIONAL_POLL          25
#define PLUGIN_REQUEST                     26
#define PLUGIN_TIME_CHANGE                 27
#define PLUGIN_MONITOR                     28
#define PLUGIN_SET_DEFAULTS                29
#define PLUGIN_GET_PACKED_RAW_DATA         30 // Return all data in a compact binary format specific for that plugin.
                                              // Needs USES_PACKED_RAW_DATA
#define PLUGIN_ONLY_TIMER_IN               31
#define PLUGIN_WEBFORM_SHOW_I2C_PARAMS     32



// ********************************************************************************
//   CPlugin (Controller) function calls
// ********************************************************************************

class CPlugin {
public:

  // As these function values are also used in the timing stats, make sure there is no overlap with the PLUGIN_xxx numbering.

  enum Function {
    CPLUGIN_PROTOCOL_ADD = 40, // Called at boot for letting a controller adding itself to list of available controllers
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

  enum Function {
    NPLUGIN_PROTOCOL_ADD = 1,
    NPLUGIN_GET_DEVICENAME,
    NPLUGIN_WEBFORM_SAVE,
    NPLUGIN_WEBFORM_LOAD,
    NPLUGIN_WRITE,
    NPLUGIN_NOTIFY
  };
};


#endif // ESPEASY_PLUGIN_DEFS_H
