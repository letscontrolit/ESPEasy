
#ifdef WEBSERVER_DOWNLOAD

// ********************************************************************************
// Web Interface download page
// ********************************************************************************
void handle_download(void)
{
  checkRAM(F("handle_download"));

  if (!isLoggedIn(void)) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;

  //  TXBuffer.startStream(void);
  //  sendHeadandTail_stdtemplate(void);


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

  if (systemTimePresent(void))
  {
    str += getDateTimeString('\0', '\0', '\0');
  }
  str += F(".dat");

  WebServer.sendHeader(F("Content-Disposition"), str);
  WebServer.streamFile(dataFile, F("application/octet-stream"));
  dataFile.close(void);
}

#endif // ifdef WEBSERVER_DOWNLOAD
