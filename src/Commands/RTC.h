#ifndef COMMAND_RTC_H
#define COMMAND_RTC_H


bool Command_RTC_Clear(struct EventStruct *event, const char* Line)
{
  bool success = true;
  initRTC();
  return success;
}

bool Command_RTC_resetFlashWriteCounter(struct EventStruct *event, const char* Line)
{
  bool success = true;
  RTC.flashDayCounter = 0;
  return success;
}

#endif // COMMAND_RTC_H