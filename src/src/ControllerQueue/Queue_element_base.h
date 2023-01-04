#ifndef CONTROLLERQUEUE_QUEUE_ELEMENT_BASE_H
#define CONTROLLERQUEUE_QUEUE_ELEMENT_BASE_H


#include "../../ESPEasy_common.h"

#include "../DataStructs/UnitMessageCount.h"
#include "../Globals/CPlugins.h"

/*********************************************************************************************\
* Base class for all controller queue elements
\*********************************************************************************************/
class Queue_element_base {
public:

  virtual ~Queue_element_base();

  virtual size_t                    getSize() const = 0;

  virtual bool                      isDuplicate(const Queue_element_base& other) const = 0;

  virtual const UnitMessageCount_t* getUnitMessageCount() const = 0;
  virtual UnitMessageCount_t      * getUnitMessageCount()       = 0;

  unsigned long _timestamp         = millis();
  controllerIndex_t _controller_idx = INVALID_CONTROLLER_INDEX;
  taskIndex_t _taskIndex            = INVALID_TASK_INDEX;

  // Call PLUGIN_PROCESS_CONTROLLER_DATA which may process the data.
  // Typical use case is dumping large data which would otherwise take up lot of RAM.
  bool _call_PLUGIN_PROCESS_CONTROLLER_DATA = false;

  // Some formatting of values can be done when actually sending it.
  // This may require less RAM than keeping formatted strings in memory
  bool _processByController = false;
};

#endif // ifndef CONTROLLERQUEUE_QUEUE_ELEMENT_BASE_H
