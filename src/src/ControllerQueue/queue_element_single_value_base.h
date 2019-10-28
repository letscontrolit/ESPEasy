#ifndef CONTROLQUEUE_QUEUE_ELEMENT_SINGLE_VALUE_BASE_H
#define CONTROLQUEUE_QUEUE_ELEMENT_SINGLE_VALUE_BASE_H


#include "../../ESPEasy_common.h"
#include "../DataStructs/ESPEasyLimits.h"

struct EventStruct;

/*********************************************************************************************\
* Base class for controllers that only send a single value per request and thus needs to
* keep track of the number of values already sent.
\*********************************************************************************************/
class queue_element_single_value_base {
public:

  queue_element_single_value_base();

  queue_element_single_value_base(const struct EventStruct *event,
                                  byte                      value_count);

  bool   checkDone(bool succesfull) const;

  size_t getSize() const;

  String txt[VARS_PER_TASK];
  int controller_idx      = 0;
  byte TaskIndex          = 0;
  int idx                 = 0;
  mutable byte valuesSent = 0; // Value must be set by const function checkDone()
  byte valueCount         = 0;
};


#endif // CONTROLQUEUE_QUEUE_ELEMENT_SINGLE_VALUE_BASE_H
