#ifndef CONTROLLERQUEUE_C015_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C015_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../Globals/CPlugins.h"
#include "../Globals/Plugins.h"

struct EventStruct;

#ifdef USES_C015

/*********************************************************************************************\
* C015_queue_element for queueing requests for 015: Blynk
* Using queue_element_single_value_base
\*********************************************************************************************/

class C015_queue_element {
public:

  C015_queue_element();

  C015_queue_element(const struct EventStruct *event, byte value_count);

  bool   checkDone(bool succesfull) const;

  size_t getSize() const;

  bool isDuplicate(const C015_queue_element& other) const;

  String txt[VARS_PER_TASK];
  int vPin[VARS_PER_TASK]          = { 0 };
  int idx                          = 0;
  unsigned long _timestamp         = millis();
  taskIndex_t TaskIndex            = INVALID_TASK_INDEX;
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;
  mutable byte valuesSent          = 0; // Value must be set by const function checkDone()
  byte valueCount                  = 0;
};

#endif //USES_C015


#endif // CONTROLLERQUEUE_C015_QUEUE_ELEMENT_H
