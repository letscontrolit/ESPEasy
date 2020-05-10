#ifndef DATASTRUCTS_ESPEASY_NOW_MESSAGE_HEADER_STRUCT_H
#define DATASTRUCTS_ESPEASY_NOW_MESSAGE_HEADER_STRUCT_H

/*********************************************************************************************\
* ESPEasy_now_message_struct
\*********************************************************************************************/

#include "../Globals/ESPEasy_now_state.h"
#ifdef USES_ESPEASY_NOW

# include <Arduino.h>

# include "../DataStructs/DeviceStruct.h"
# include "../DataStructs/ESPEasyLimits.h"
# include "../Globals/Plugins.h"


# define ESPEASY_NOW_HEADER_VERSION  1


struct ESPEasy_now_message_header_struct {
  enum class message_t : byte {
    Acknowledgement         = 0,
    TaskName                = 1, // Just the name of the task (null terminated string)
    TaskValueName_TaskValue = 2, // Name of the Task value (null terminated string) + string formatted value
    taskValue               = 3, // Just the string formatted value (can be next part of the value, for SENSOR_TYPE_STRING)
  };

  ESPEasy_now_message_header_struct();

  const byte header_version = ESPEASY_NOW_HEADER_VERSION;                // To be used later to detect newer versions
  const byte header_length  = sizeof(ESPEasy_now_message_header_struct); // Length of the header
  message_t  message_type   = message_t::Acknowledgement;
  byte       cur_message_nr = 0;                                         // Current message number (start at 0, always <
                                                                         // total_nr_messages)
  byte total_nr_messages = 1;                                            // Total number of messages needed to transfer all information
                                                                         // (can be >1 for
                                                                         // SENSOR_TYPE_STRING)
  byte cur_payload_size    = 0;                                          // The size of the payload in this message.
  byte VType               = SENSOR_TYPE_NONE;                           // Type of value the plugin will return. e.g. SENSOR_TYPE_STRING
  byte ValueCount          = 0;                                          // The number of output values of a plugin.
  byte cur_taskvalue_index = 0;                                          // Describing which task value is sent (start at 0, always <
                                                                         // ValueCount)
  taskIndex_t taskIndex = INVALID_TASK_INDEX;                            // Task index of the data origin when creating message
  uint16_t    plugin_id = INVALID_PLUGIN_ID;                             // FIXME TD-er: Change to pluginID_t when that one is switched
                                                                         // to 16 bit value
};

#endif // ifdef USES_ESPEASY_NOW

#endif // DATASTRUCTS_ESPEASY_NOW_MESSAGE_HEADER_STRUCT_H
