#ifndef CONTROLLERQUEUE_MQTT_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_MQTT_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"
#include "../Globals/CPlugins.h"


/*********************************************************************************************\
* MQTT_queue_element for all MQTT base controllers
\*********************************************************************************************/
class MQTT_queue_element {
public:

  MQTT_queue_element();

  explicit MQTT_queue_element(int           ctrl_idx,
                              const String& topic,
                              const String& payload,
                              bool          retained);

  size_t getSize() const;

  String _topic;
  String _payload;
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;
  bool _retained                   = false;
};


#endif // CONTROLLERQUEUE_MQTT_QUEUE_ELEMENT_H
