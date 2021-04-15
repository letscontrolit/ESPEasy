#ifndef COMMAND_TIMER_H
#define COMMAND_TIMER_H

class String;

String Command_Timer_Set (struct EventStruct *event, const char* Line);
String Command_Timer_Set_ms (struct EventStruct *event, const char* Line);
String Command_Loop_Timer_Set (struct EventStruct *event, const char* Line);
String Command_Loop_Timer_Set_ms (struct EventStruct *event, const char* Line);
String Command_Timer_Pause (struct EventStruct *event, const char* Line);
String Command_Timer_Resume (struct EventStruct *event, const char* Line);
String Command_Delay (struct EventStruct *event, const char* Line);

#endif // COMMAND_TIMER_H
