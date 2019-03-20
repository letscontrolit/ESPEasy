#ifndef COMMAND_BLYNK_C014_H
#define COMMAND_BLYNK_C014_H

#ifdef USES_C014

String Command_Blynk_Set(struct EventStruct *event, const char* Line)
{
  return Command_Blynk_Set_c014(event,Line);
}
#endif

#endif // COMMAND_BLYNK_C014_H
