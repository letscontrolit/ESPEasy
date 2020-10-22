#include "../Commands/SDCARD.h"

#include "../../ESPEasy_common.h"
#include "../Commands/Common.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/Settings.h"




#ifdef FEATURE_SD

#include <SD.h>


void printDirectory(File dir, int numTabs)
{
  while (true) {
    File entry = dir.openNextFile();

    if (!entry) {
      // no more files
      break;
    }

    for (uint8_t i = 0; i < numTabs; i++) {
      serialPrint("\t");
    }
    serialPrint(entry.name());

    if (entry.isDirectory()) {
      serialPrintln("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      serialPrint("\t\t");
      serialPrintln(String(entry.size(), DEC));
    }
    entry.close();
  }
}


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
  // FIXME TD-er: This one is not using parseString* function
  String fname = Line;
  fname = fname.substring(9);
  String result = F("Removing:");
  result += fname.c_str();
  SD.remove((char*)fname.c_str());
  return return_result(event, result);
}
#endif