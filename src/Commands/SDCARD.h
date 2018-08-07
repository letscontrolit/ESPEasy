#ifndef COMMAND_SDCARD_H
#define COMMAND_SDCARD_H


#ifdef FEATURE_SD
String Command_SD_LS(struct EventStruct *event, const char* Line)
{
  File root = SD.open("/");
  root.rewindDirectory();
  printDirectory(root, 0);
  root.close();
  return return_see_serial(event);
}

String Command_SD_Remove(struct EventStruct *event, const char* Line)
{
  String fname = Line;
  fname = fname.substring(9);
  String result = F("Removing:");
  result += fname.c_str();
  SD.remove((char*)fname.c_str());
  return return_result(event, result);
}
#endif

#endif // COMMAND_SDCARD_H
