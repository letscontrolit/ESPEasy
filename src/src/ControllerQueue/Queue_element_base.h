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
  virtual UnitMessageCount_t      * getUnitMessageCount() = 0;

  unsigned long _timestamp         = millis();
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;

};

#endif
