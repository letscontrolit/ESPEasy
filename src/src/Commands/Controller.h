#ifndef COMMAND_CONTROLLR_H
#define COMMAND_CONTROLLR_H

class String;

String Command_Controller_Disable(struct EventStruct *event, const char* Line);
String Command_Controller_Enable(struct EventStruct *event, const char* Line);



#endif // COMMAND_CONTROLLR_H