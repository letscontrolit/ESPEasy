#ifndef COMMAND_NETWORKS_H
#define COMMAND_NETWORKS_H

class String;


String Command_AccessInfo_Ls(struct EventStruct *event, const char* Line);
String Command_AccessInfo_Clear (struct EventStruct *event, const char* Line);
String Command_DNS (struct EventStruct *event, const char* Line);
String Command_Gateway (struct EventStruct *event, const char* Line);
String Command_IP (struct EventStruct *event, const char* Line);
String Command_Subnet (struct EventStruct *event, const char* Line);

#endif // COMMAND_NETWORKS_H
