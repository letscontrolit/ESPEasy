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

bool Command_MenInfo(struct EventStruct *event, const char* Line)
{
  bool success = true;
  Serial.print(F("SecurityStruct         : "));
  Serial.println(sizeof(SecuritySettings));
  Serial.print(F("SettingsStruct         : "));
  Serial.println(sizeof(Settings));
  Serial.print(F("ExtraTaskSettingsStruct: "));
  Serial.println(sizeof(ExtraTaskSettings));
  Serial.print(F("DeviceStruct: "));
  Serial.println(sizeof(Device));
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
    Settings.SerialLogLevel = event->Par1;
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