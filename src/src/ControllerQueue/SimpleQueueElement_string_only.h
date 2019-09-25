#ifndef CONTROLLERQUEUE_SIMPLE_QUEUE_ELEMENT_STRING_ONLY_H
#define CONTROLLERQUEUE_SIMPLE_QUEUE_ELEMENT_STRING_ONLY_H

#include "../../ESPEasy_common.h"

/*********************************************************************************************\
* Simple queue element, only storing controller index and some String
\*********************************************************************************************/
class simple_queue_element_string_only {
public:

  simple_queue_element_string_only();

  simple_queue_element_string_only(int           ctrl_idx,
                                   const String& req);

  size_t getSize() const;

  int controller_idx = 0;
  String txt;
};


#endif // CONTROLLERQUEUE_SIMPLE_QUEUE_ELEMENT_STRING_ONLY_H
