#include "../ControllerQueue/Queue_element_base.h"

Queue_element_base::Queue_element_base() :
  _controller_idx(INVALID_CONTROLLER_INDEX),
  _taskIndex(INVALID_TASK_INDEX),
  _call_PLUGIN_PROCESS_CONTROLLER_DATA(false),
  _processByController(false)
{
  _timestamp = millis();
}

Queue_element_base::~Queue_element_base() {}
