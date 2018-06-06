char* ramtest;

//Reads a string from a stream until a terminator-character.
//We make sure we're not reading more than maxSize bytes and we're not busy for longer than timeout mS.

void ExecuteCommand(byte source, const char *Line)
{
  checkRAM(F("ExecuteCommand"));
  String status = "";
  boolean success = false;
  char TmpStr1[INPUT_COMMAND_SIZE];
  TmpStr1[0] = 0;
  char cmd[INPUT_COMMAND_SIZE];   
  cmd[0] = 0;
  struct EventStruct TempEvent;
  TempEvent.Source = source; 
  GetArgv(Line, cmd, 1);
  if (GetArgv(Line, TmpStr1, 2)) TempEvent.Par1 = str2int(TmpStr1);
  if (GetArgv(Line, TmpStr1, 3)) TempEvent.Par2 = str2int(TmpStr1);
  if (GetArgv(Line, TmpStr1, 4)) TempEvent.Par3 = str2int(TmpStr1);
  if (GetArgv(Line, TmpStr1, 5)) TempEvent.Par4 = str2int(TmpStr1);
  if (GetArgv(Line, TmpStr1, 6)) TempEvent.Par5 = str2int(TmpStr1);
  
  success = cmdManager.Execute((char*)&cmd[0], &TempEvent, Line);
  yield();

  if (success)
    status += F("\nOk");
  else
    status += F("\nUnknown command!");
  SendStatus(source, status);
  yield();
}

#ifdef FEATURE_SD
void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
#endif
