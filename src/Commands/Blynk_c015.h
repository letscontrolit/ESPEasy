#ifndef COMMAND_BLYNK_C015_H
#define COMMAND_BLYNK_C015_H

#ifdef USES_BLYNK
#ifdef USES_C015

String Command_Blynk_Set(struct EventStruct *event, const char* Line)
{
  return Command_Blynk_Set_c015(event,Line);
}
#endif //USES_C015
#endif //USES_BLYNK

#endif // COMMAND_BLYNK_C015_H
