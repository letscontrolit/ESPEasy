#ifndef COMMAND_SDCARD_H
#define COMMAND_SDCARD_H


#ifdef FEATURE_SD
bool Command_SD_LS(struct EventStruct *event, const char* Line)
{
  bool success = true;
  success = true;
  File root = SD.open("/");
  root.rewindDirectory();
  printDirectory(root, 0);
  root.close();
  return success;
}
bool Command_SD_Remove(struct EventStruct *event, const char* Line)
{
  success = true;
  String fname = Line;
  fname = fname.substring(9);
  Serial.print(F("Removing:"));
  Serial.println(fname.c_str());
  SD.remove((char*)fname.c_str());
  return success;
}
#endif

#endif // COMMAND_SDCARD_H