#ifndef COMMAND_DIAGNOSTIC_H
#define COMMAND_DIAGNOSTIC_H


bool Command_Lowmem(struct EventStruct *event, const char* Line)
{
  bool success = true;
  Serial.print(lowestRAM);
  Serial.print(F(" : "));
  Serial.println(lowestRAMfunction);
  return success;
}

char* ramtest;
bool Command_Malloc(struct EventStruct *event, const char* Line)
{
  bool success = true;
  ramtest = (char *)malloc(event->Par1);
  return success;
}

bool Command_SysLoad(struct EventStruct *event, const char* Line)
{
  bool success = true;
  Serial.print(getCPUload());
  Serial.print(F("% (LC="));
  Serial.print(getCPUload());
  Serial.println(F(")"));
  return success;
}

bool Command_SerialFloat(struct EventStruct *event, const char* Line)
{
  bool success = true;
  pinMode(1, INPUT);
  pinMode(3, INPUT);
  delay(60000);
  return success;
}

bool Command_MemInfo(struct EventStruct *event, const char* Line)
{
  bool success = true;
  Serial.print(F("SecurityStruct         | "));
  Serial.println(sizeof(SecuritySettings));
  Serial.print(F("SettingsStruct         | "));
  Serial.println(sizeof(Settings));
  Serial.print(F("ExtraTaskSettingsStruct| "));
  Serial.println(sizeof(ExtraTaskSettings));
  Serial.print(F("DeviceStruct           | "));
  Serial.println(sizeof(Device));
  return success;
}

bool Command_MemInfo_detail(struct EventStruct *event, const char* Line)
{
  bool success = true;
  showSettingsFileLayout = true;
  Command_MemInfo(event, Line);
  for (int st = 0; st < SettingsType_MAX; ++st) {
    SettingsType settingsType = static_cast<SettingsType>(st);
    int max_index, offset, max_size;
    int struct_size = 0;
    Serial.println();
    Serial.print(getSettingsTypeString(settingsType));
    Serial.println(F(" | start | end | max_size | struct_size"));
    Serial.println(F("--- | --- | --- | --- | ---"));
    getSettingsParameters(settingsType, 0, max_index, offset, max_size, struct_size);
    for (int i = 0; i < max_index; ++i) {
      getSettingsParameters(settingsType, i, offset, max_size);
      Serial.print(i);
      Serial.print('|');
      Serial.print(offset);
      Serial.print('|');
      Serial.print(offset + max_size - 1);
      Serial.print('|');
      Serial.print(max_size);
      Serial.print('|');
      Serial.println(struct_size);
    }
  }

  return success;
}

bool Command_Background(struct EventStruct *event, const char* Line)
{
  bool success = true;
  unsigned long timer = millis() + event->Par1;
  Serial.println(F("start"));
  while (!timeOutReached(timer))
    backgroundtasks();
  Serial.println(F("end"));
  return success;
}


bool Command_Debug(struct EventStruct *event, const char* Line)
{
  char TmpStr1[INPUT_COMMAND_SIZE];
  if (GetArgv(Line, TmpStr1, 2)) {
    setLogLevelFor(LOG_TO_SERIAL, event->Par1);
  }
  else{
    Serial.println();
    Serial.print(F("Serial debug level: "));
    Serial.println(Settings.SerialLogLevel);
  }
  return true;
}

bool Command_logentry(struct EventStruct *event, const char* Line)
{
  return true;
}

#endif // COMMAND_DIAGNOSTIC_H
