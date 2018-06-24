#ifndef COMMAND_TIME_H
#define COMMAND_TIME_H


bool Command_NTPHost (struct EventStruct *event, const char* Line)
{
  return Command_GetORSetString(F("NTPHost:"), 
    Line, 
    Settings.NTPHost,
    sizeof(Settings.NTPHost),
    1);
}

bool Command_useNTP (struct EventStruct *event, const char* Line)
{
  char TmpStr1[INPUT_COMMAND_SIZE];
  if (GetArgv(Line, TmpStr1, 2)) {
    Settings.UseNTP = event->Par1;
  }
  else{
    Serial.println();
    Serial.print(F("UseNTP:"));
    Serial.println(Settings.UseNTP ? "true" : "false" );
  }
  return true;
}

bool Command_TimeZone (struct EventStruct *event, const char* Line)
{
  char TmpStr1[INPUT_COMMAND_SIZE];
  if (GetArgv(Line, TmpStr1, 2)) {
    Settings.TimeZone = event->Par1;
  }
  else{
    Serial.println();
    Serial.print(F("TimeZone:"));
    Serial.println(Settings.TimeZone);
  }
  return true;
}

bool Command_DST (struct EventStruct *event, const char* Line)
{
  char TmpStr1[INPUT_COMMAND_SIZE];
  if (GetArgv(Line, TmpStr1, 2)) {
    Settings.DST = event->Par1;
  }
  else{
    Serial.println();
    Serial.print(F("DST:"));
    Serial.println(Settings.DST ? "true" : "false");
  }
  return true;
}

#endif // COMMAND_TIME_H