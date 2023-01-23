#ifndef CONTROLLER_CONFIG_C019_CONFIG_H
#define CONTROLLER_CONFIG_C019_CONFIG_H

#include "src/Helpers/_CPlugin_Helper.h"

#ifdef USES_C019

# define C019_MQTT_TOPIC_LENGTH 128

struct C019_ConfigStruct
{
  C019_ConfigStruct() = default;

  void validate();

  void reset();

  // Send all to the web interface
  void webform_load();

  // Collect all data from the web interface
  void webform_save();


  uint8_t configVersion   = 1; // Format version of the stored data
  uint8_t dummy_not_used  = 0; // For 32-bit alignment
  uint8_t dummy_not_used1 = 0; // For 32-bit alignment
  uint8_t dummy_not_used2 = 0; // For 32-bit alignment

  union {
    struct {
      uint32_t forwardMQTT        : 1;
      uint32_t filterMQTT_forward : 1;


      uint32_t notUsed : 30; // All bits should add up to 32
    };

    uint32_t variousBits = 0;
  };
  int8_t wifiChannel = -1;

  // MQTT filter options
  taskIndex_t       filterTaskIndex                             = INVALID_TASK_INDEX;       // Task index to use for filtering
  controllerIndex_t forwardControllerIdx                        = INVALID_CONTROLLER_INDEX; // Controller index to forward filtered data to
  int8_t            filterTopic_startindex                      = -1;                       //
  int8_t            filterTopic_endindex                        = -1;
  char              filterPublishPrefix[C019_MQTT_TOPIC_LENGTH] = { 0 };
  char              filterSubscribe[C019_MQTT_TOPIC_LENGTH]     = { 0 };
};


#endif // ifdef USES_C019

#endif // ifndef CONTROLLER_CONFIG_C019_CONFIG_H
