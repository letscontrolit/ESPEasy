#ifndef CONTROLLER_CONFIG_C019_CONFIG_H
#define CONTROLLER_CONFIG_C019_CONFIG_H

#include "src/Helpers/_CPlugin_Helper.h"

#ifdef USES_C019

# include <vector>

# define C019_MQTT_TOPIC_LENGTH 128

struct C019_ForwardFiltering
{
  void fromStringArray(String  strings[],
                       uint8_t filterNr);
  void toStringArray(String  strings[],
                     uint8_t filterNr) const;

  taskIndex_t taskIndex = INVALID_TASK_INDEX;
  String      matchTopic;
};

struct C019_ConfigStruct
{
  C019_ConfigStruct() = default;

  void        validate();

  void        reset();

  void        init(struct EventStruct *event);

  // Send all to the web interface
  void        webform_load(struct EventStruct *event);

  // Collect all data from the web interface
  void        webform_save(struct EventStruct *event);

  taskIndex_t matchTopic(const String& topic) const;


  uint8_t           configVersion        = 1;                        // Format version of the stored data
  uint8_t           nrTaskFilters        = 0;                        // Number of task/topics to configure for filtering
  int8_t            wifiChannel          = -1;
  controllerIndex_t forwardControllerIdx = INVALID_CONTROLLER_INDEX; // Controller index to forward filtered data to

  union {
    struct {
      uint32_t forwardMQTT        : 1;
      uint32_t filterMQTT_forward : 1;


      uint32_t notUsed : 30; // All bits should add up to 32
    };

    uint32_t variousBits = 0;
  };

  uint32_t reserved = 0; // Need to have the start of 'filters' at the same position.

  std::vector<C019_ForwardFiltering>filters;
};


typedef std::shared_ptr<C019_ConfigStruct> C019_ConfigStruct_ptr;

#endif // ifdef USES_C019

#endif // ifndef CONTROLLER_CONFIG_C019_CONFIG_H
