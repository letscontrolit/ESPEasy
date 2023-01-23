#ifndef CONTROLLERQUEUE_QUEUE_ELEMENT_BASE_H
#define CONTROLLERQUEUE_QUEUE_ELEMENT_BASE_H


#include "../../ESPEasy_common.h"

#include "../DataStructs/MessageRouteInfo.h"
#include "../Globals/CPlugins.h"

/*********************************************************************************************\
* Base class for all controller queue elements
\*********************************************************************************************/
class Queue_element_base {
public:
  Queue_element_base();

  virtual ~Queue_element_base();

  virtual size_t                    getSize() const = 0;

  virtual bool                      isDuplicate(const Queue_element_base& other) const = 0;

  virtual const MessageRouteInfo_t* getMessageRouteInfo() const = 0;
  virtual MessageRouteInfo_t      * getMessageRouteInfo()       = 0;

  unsigned long _timestamp;
  controllerIndex_t _controller_idx;
  taskIndex_t _taskIndex;

  // Call PLUGIN_PROCESS_CONTROLLER_DATA which may process the data.
  // Typical use case is dumping large data which would otherwise take up lot of RAM.
  bool _call_PLUGIN_PROCESS_CONTROLLER_DATA;

  // Call PLUGIN_FILTEROUT_CONTROLLER_DATA which will determine whether the message should be further processed or not.
  // Typical use case is forwarding MQTT messages via ESPEasy-NOW mesh.
  bool _call_PLUGIN_FILTEROUT_CONTROLLER_DATA;

  // Some formatting of values can be done when actually sending it.
  // This may require less RAM than keeping formatted strings in memory
  bool _processByController;
  
};

#endif // ifndef CONTROLLERQUEUE_QUEUE_ELEMENT_BASE_H
