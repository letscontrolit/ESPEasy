#include "../WebServer/DownloadPage.h"

#ifdef WEBSERVER_DOWNLOAD

#include "../WebServer/ESPEasy_WebServer.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/Settings.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/StringGenerator_System.h"


// ********************************************************************************
// Web Interface download page
// ********************************************************************************
void handle_download()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_download"));
  #endif

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;

  //  TXBuffer.startStream();
  //  sendHeadandTail_stdtemplate(_HEAD);


  fs::File dataFile = tryOpenFile(getFileName(FileType::CONFIG_DAT), "r");

  if (!dataFile) {
    return;
  }

  String str = F("attachment; filename=config_");
  str += Settings.getName();
  str += F("_U");
  str += Settings.Unit;
  str += F("_Build");
  str += getSystemBuildString();
  str += '_';

  if (node_time.systemTimePresent())
  {
    str += node_time.getDateTimeString('\0', '\0', '\0');
  }
  str += F(".dat");

  sendHeader(F("Content-Disposition"), str);
  web_server.streamFile(dataFile, F("application/octet-stream"));
  dataFile.close();
}

#endif // ifdef WEBSERVER_DOWNLOAD
