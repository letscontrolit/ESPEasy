#ifndef COMMAND_TASKS_H
#define COMMAND_TASKS_H

#include "../Globals/Plugins.h"

class String;

String Command_Task_Clear(struct EventStruct *event, const char* Line);
String Command_Task_ClearAll(struct EventStruct *event, const char* Line);
String Command_Task_Disable(struct EventStruct *event, const char* Line);
String Command_Task_Enable(struct EventStruct *event, const char* Line);
String Command_Task_ValueSet(struct EventStruct *event, const char* Line);
String Command_Task_ValueToggle(struct EventStruct *event, const char* Line);
String Command_Task_ValueSetAndRun(struct EventStruct *event, const char* Line);
String Command_Task_Run(struct EventStruct *event, const char* Line);
String Command_Task_RemoteConfig(struct EventStruct *event, const char* Line);

//bool validTaskVars(struct EventStruct *event, taskIndex_t& taskIndex, unsigned int& varNr);

#endif // COMMAND_TASKS_H
