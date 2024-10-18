#include "../Commands/ESPEasy_Now_cmd.h"


#include "../../ESPEasy_common.h"

#ifdef USES_ESPEASY_NOW

#include "../Commands/Common.h"
#include "../Globals/ESPEasy_now_handler.h"

const __FlashStringHelper * Command_ESPEasy_Now_Disable(struct EventStruct *event, const char *Line)
{
  temp_disable_EspEasy_now_timer = millis() + (5*60*1000);
  ESPEasy_now_handler.end();
  return return_command_success_flashstr();
}

const __FlashStringHelper * Command_ESPEasy_Now_Enable(struct EventStruct *event, const char *Line)
{
  // Do not set to 0, but to some moment that will be considered passed when checked the next time.
  temp_disable_EspEasy_now_timer = millis();
  return return_command_success_flashstr();
}

#endif // ifdef USES_ESPEASY_NOW
