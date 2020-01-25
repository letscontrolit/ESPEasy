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


// ********************************************************************************
//   CPlugin (Controller) function calls
// ********************************************************************************

// Make sure the CPLUGIN_* does not overlap PLUGIN_*
#define CPLUGIN_PROTOCOL_ADD               41 // Called at boot for letting a controller adding itself to list of available controllers
#define CPLUGIN_PROTOCOL_TEMPLATE          42
#define CPLUGIN_PROTOCOL_SEND              43
#define CPLUGIN_PROTOCOL_RECV              44
#define CPLUGIN_GET_DEVICENAME             45
#define CPLUGIN_WEBFORM_SAVE               46
#define CPLUGIN_WEBFORM_LOAD               47
#define CPLUGIN_GET_PROTOCOL_DISPLAY_NAME  48
#define CPLUGIN_TASK_CHANGE_NOTIFICATION   49
#define CPLUGIN_INIT                       50
#define CPLUGIN_UDP_IN                     51
#define CPLUGIN_FLUSH                      52 // Force offloading data stored in buffers, called before sleep/reboot
#define CPLUGIN_TEN_PER_SECOND             53 // Called 10x per second (typical for checking new data instead of waiting)
#define CPLUGIN_INIT_ALL                   54
#define CPLUGIN_EXIT                       55


// new messages for autodiscover controller plugins (experimental) i.e. C014
#define CPLUGIN_GOT_CONNECTED              56 // call after connected to mqtt server to publich device autodicover features
#define CPLUGIN_GOT_INVALID                57 // should be called before major changes i.e. changing the device name to clean up data on the controller. !ToDo
#define CPLUGIN_INTERVAL                   58 // call every interval loop
#define CPLUGIN_ACKNOWLEDGE                59 // call for sending acknowledges !ToDo done by direct function call in PluginCall() for now.

#define CPLUGIN_WEBFORM_SHOW_HOST_CONFIG   60 // Used for showing host information for the controller.




// ********************************************************************************
//   NPlugin (Notification) function calls
// ********************************************************************************
#define NPLUGIN_PROTOCOL_ADD                1
#define NPLUGIN_GET_DEVICENAME              2
#define NPLUGIN_WEBFORM_SAVE                3
#define NPLUGIN_WEBFORM_LOAD                4
#define NPLUGIN_WRITE                       5
#define NPLUGIN_NOTIFY                      6
#define NPLUGIN_NOT_FOUND                 255


#endif // ESPEASY_PLUGIN_DEFS_H
