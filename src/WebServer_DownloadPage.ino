
// ********************************************************************************
// Web Interface download page
// ********************************************************************************
void handle_download()
{
  checkRAM(F("handle_download"));

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;

  //  TXBuffer.startStream();
  //  sendHeadandTail_stdtemplate();


  fs::File dataFile = tryOpenFile(F(FILE_CONFIG), "r");

  if (!dataFile) {
    return;
  }

  String str = F("attachment; filename=config_");
  str += Settings.Name;
  str += "_U";
  str += Settings.Unit;
  str += F("_Build");
  str += BUILD;
  str += '_';

  if (systemTimePresent())
  {
    str += getDateTimeString('\0', '\0', '\0');
  }
  str += F(".dat");

  WebServer.sendHeader(F("Content-Disposition"), str);
  WebServer.streamFile(dataFile, F("application/octet-stream"));
  dataFile.close();
}
