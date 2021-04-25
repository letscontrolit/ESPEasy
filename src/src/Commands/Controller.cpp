#include "../Commands/Controller.h"


#include "../../ESPEasy_common.h"


#include "../Commands/Common.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../DataTypes/ControllerIndex.h"

#include "../Globals/CPlugins.h"

#include "../Helpers/Misc.h"

//      controllerIndex = (event->Par1 - 1);   Par1 is here for 1 ... CONTROLLER_MAX
bool validControllerVar(struct EventStruct *event, controllerIndex_t& controllerIndex)
{
  if (event->Par1 <= 0) { return false; }
  controllerIndex = static_cast<controllerIndex_t>(event->Par1 - 1);
  return validControllerIndex(controllerIndex);
}

String Command_Controller_Disable(struct EventStruct *event, const char *Line)
{
  controllerIndex_t controllerIndex;

  if (validControllerVar(event, controllerIndex) && setControllerEnableStatus(controllerIndex, false)) {
    return return_command_success();
  }
  return return_command_failed();
}

String Command_Controller_Enable(struct EventStruct *event, const char *Line)
{
  controllerIndex_t controllerIndex;

  if (validControllerVar(event, controllerIndex) && setControllerEnableStatus(controllerIndex, true)) {
    return return_command_success();
  }
  return return_command_failed();
}
