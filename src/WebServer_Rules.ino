// #define WEBSERVER_RULES_DEBUG


// ********************************************************************************
// Web Interface rules page
// ********************************************************************************
void handle_rules() {
  checkRAM(F("handle_rules"));

  if (!isLoggedIn() || !Settings.UseRules) { return; }
  navMenuIndex = MENU_INDEX_RULES;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();
  static byte currentSet = 1;

  const byte rulesSet = getFormItemInt(F("set"), 1);

  #if defined(ESP8266)
  String fileName = F("rules");
  #endif // if defined(ESP8266)
  #if defined(ESP32)
  String fileName = F("/rules");
  #endif // if defined(ESP32)
  fileName += rulesSet;
  fileName += F(".txt");


  checkRAM(F("handle_rules"));


  if (WebServer.args() > 0)
  {
    String log = F("Rules : Save rulesSet: ");
    log += rulesSet;
    log += F(" currentSet: ");
    log += currentSet;

    if (currentSet == rulesSet) // only save when the dropbox was not used to change set
    {
      String rules = WebServer.arg(F("rules"));
      log += F(" rules.length(): ");
      log += rules.length();

      if (rules.length() > RULES_MAX_SIZE) {
        TXBuffer += F("<span style=\"color:red\">Data was not saved, exceeds web editor limit!</span>");
      }
      else
      {
        // if (RTC.flashDayCounter > MAX_FLASHWRITES_PER_DAY)
        // {
        //   String log = F("FS   : Daily flash write rate exceeded! (powercyle to reset this)");
        //   addLog(LOG_LEVEL_ERROR, log);
        //   TXBuffer += F("<span style=\"color:red\">Error saving to flash!</span>");
        // }
        // else
        // {
        fs::File f = tryOpenFile(fileName, "w");

        if (f)
        {
          log += F(" Write to file: ");
          log += fileName;
          f.print(rules);
          f.close();

          // flashCount();
        }

        // }
      }
    }
    else // changed set, check if file exists and create new
    {
      if (!SPIFFS.exists(fileName))
      {
        log += F(" Create new file: ");
        log += fileName;
        fs::File f = tryOpenFile(fileName, "w");

        if (f) { f.close(); }
      }
    }
    addLog(LOG_LEVEL_INFO, log);

    log = F(" Webserver args:");

    for (int i = 0; i < WebServer.args(); ++i) {
      log += ' ';
      log += i;
      log += F(": '");
      log += WebServer.argName(i);
      log += F("' length: ");
      log += WebServer.arg(i).length();
    }
    addLog(LOG_LEVEL_INFO, log);
  }

  if (rulesSet != currentSet) {
    currentSet = rulesSet;
  }

  TXBuffer += F("<form name = 'frmselect' method = 'post'>");
  html_table_class_normal();
  html_TR();
  html_table_header(F("Rules"));

  byte   choice = rulesSet;
  String options[RULESETS_MAX];
  int    optionValues[RULESETS_MAX];

  for (byte x = 0; x < RULESETS_MAX; x++)
  {
    options[x]      = F("Rules Set ");
    options[x]     += x + 1;
    optionValues[x] = x + 1;
  }

  html_TR_TD();
  addSelector(F("set"), RULESETS_MAX, options, optionValues, NULL, choice, true);
  addHelpButton(F("Tutorial_Rules"));

  // load form data from flash

  int size   = 0;
  fs::File f = tryOpenFile(fileName, "r");

  if (f)
  {
    size = f.size();

    if (size > RULES_MAX_SIZE) {
      TXBuffer += F("<span style=\"color:red\">Filesize exceeds web editor limit!</span>");
    }
    else
    {
      html_TR_TD(); TXBuffer += F("<textarea name='rules' rows='30' wrap='off'>");

      while (f.available())
      {
        String c((char)f.read());
        htmlEscape(c);
        TXBuffer += c;
      }
      TXBuffer += F("</textarea>");
    }
    f.close();
  }

  html_TR_TD(); TXBuffer += F("Current size: ");
  TXBuffer               += size;
  TXBuffer               += F(" characters (Max ");
  TXBuffer               += RULES_MAX_SIZE;
  TXBuffer               += ')';

  addFormSeparator(2);

  html_TR_TD();
  addSubmitButton();
  addButton(fileName, F("Download to file"));
  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();

  checkRuleSets();
}

// ********************************************************************************
// Web Interface rules page  (NEW)
// ********************************************************************************
void handle_rules_new() {
  if (!isLoggedIn() || !Settings.UseRules) { return; }

  if (!clientIPallowed()) { return; }

  if (Settings.OldRulesEngine())
  {
    handle_rules();
    return;
  }
  checkRAM(F("handle_rules"));
  navMenuIndex = 5;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"), _HEAD);

  // define macro
  #if defined(ESP8266)
  String rootPath = F("rules");
  #endif // if defined(ESP8266)
  #if defined(ESP32)

  String rootPath = F("/rules");
  #endif // if defined(ESP32)

  // Pagionation of rules list
  const int rulesListPageSize = 25;
  int startIdx                = 0;

  String fstart = WebServer.arg(F("start"));

  if (fstart.length() > 0)
  {
    startIdx = atoi(fstart.c_str());
  }
  int endIdx = startIdx + rulesListPageSize - 1;

  // Build table header
  html_table_class_multirow();
  html_TR();
  html_table_header(F("Event Name"));
  html_table_header(F("Filename"));
  html_table_header(F("Size"));
  TXBuffer += F("<TH>Actions");
  addSaveButton(TXBuffer, F("/rules/backup"), F("Backup"));
  TXBuffer += F("</TH></TR>");

  // class StreamingBuffer buffer = TXBuffer;

  // Build table detail
  int count = -1;
  HandlerFileInfo renderDetail = [/*&buffer,*/ &count, endIdx](fileInfo fi) {
    #ifdef WEBSERVER_RULES_DEBUG
                                   Serial.print(F("Start generation of: "));
                                   Serial.println(fi.Name);
    #endif // ifdef WEBSERVER_RULES_DEBUG

                                   if (fi.isDirectory)
                                   {
                                     TXBuffer += F("<TR><TD>");
                                   }
                                   else
                                   {
                                     count++;
                                     TXBuffer += F("<TR><TD style='text-align:right'>");
                                   }

                                   // Event Name
                                   TXBuffer += FileNameToEvent(fi.Name);

                                   if (fi.isDirectory)
                                   {
                                     TXBuffer += F("</TD><TD></TD><TD></TD><TD>");
                                     addSaveButton(TXBuffer
                                                   , String(F("/rules/backup?directory=")) + URLEncode(fi.Name.c_str())
                                                   , F("Backup")
                                                   );
                                   }
                                   else
                                   {
                                     String encodedPath =  URLEncode((fi.Name + F(".txt")).c_str());

                                     // File Name
                                     TXBuffer += F("</TD><TD><a href='");
                                     TXBuffer += fi.Name;
                                     TXBuffer += F(".txt");
                                     TXBuffer += "'>";
                                     TXBuffer += fi.Name;
                                     TXBuffer += F(".txt");
                                     TXBuffer += F("</a></TD>");

                                     // File size
                                     TXBuffer += F("<TD>");
                                     TXBuffer += fi.Size;
                                     TXBuffer += F("</TD>");

                                     // Actions
                                     TXBuffer += F("<TD>");
                                     addSaveButton(TXBuffer
                                                   , String(F("/rules/backup?fileName=")) + encodedPath
                                                   , F("Backup")
                                                   );

                                     addDeleteButton(TXBuffer
                                                     , String(F("/rules/delete?fileName=")) + encodedPath
                                                     , F("Delete")
                                                     );
                                   }
                                   TXBuffer += F("</TD></TR>");
    #ifdef WEBSERVER_RULES_DEBUG
                                   Serial.print(F("End generation of: "));
                                   Serial.println(fi.Name);
    #endif // ifdef WEBSERVER_RULES_DEBUG

                                   return count <= endIdx;
                                 };


  bool hasMore = EnumerateFileAndDirectory(rootPath
                                           , startIdx
                                           , renderDetail);
  TXBuffer += F("<TR><TD>");
  addButton(F("/rules/add"), F("Add"));
  TXBuffer += F("</TD><TD></TD><TD></TD><TD></TD></TR>");
  TXBuffer += F("</table>");

  if (startIdx > 0)
  {
    int showIdx = startIdx - rulesListPageSize;

    if (showIdx < 0) { showIdx = 0; }
    addButton(TXBuffer
              , String(F("/rules?start=")) + String(showIdx)
              , F("Previous"));
  }

  if (hasMore && (count >= endIdx))
  {
    addButton(TXBuffer
              , String(F("/rules?start=")) + String(endIdx + 1)
              , F("Next"));
  }

  // TXBuffer += F("<BR><BR>");
  sendHeadandTail(F("TmplStd"), _TAIL);
  TXBuffer.endStream();
  checkRAM(F("handle_rules"));
}

void handle_rules_backup() {
  if (Settings.OldRulesEngine())
  {
    Goto_Rules_Root();
    return;
  }
  #ifdef WEBSERVER_RULES_DEBUG
  Serial.println(F("handle rules backup"));
  #endif // ifdef WEBSERVER_RULES_DEBUG

  if (!isLoggedIn() || !Settings.UseRules) { return; }

  if (!clientIPallowed()) { return; }
  checkRAM(F("handle_rules_backup"));
  String directory = WebServer.arg(F("directory"));
  String fileName  = WebServer.arg(F("fileName"));
  String error;

  if (directory.length() > 0)
  {
    HandlerFileInfo downloadHandler = [&error](fileInfo fi) {
                                        if (!fi.isDirectory)
                                        {
                                          if (!Rule_Download(fi.Name))
                                          {
                                            error += String(F("Invalid path: ")) + fi.Name;
                                          }
                                        }
                                        return true;
                                      };
    EnumerateFileAndDirectory(directory
                              , 0
                              , downloadHandler);
  }
  else if (fileName.length() > 0) {
    fileName = fileName.substring(0, fileName.length() - 4);

    if (!Rule_Download(fileName))
    {
      error = String(F("Invalid path: ")) + fileName;
    }
  }
  else
  {
    error = F("Invalid parameters");
  }

  if (error.length() > 0) {
    TXBuffer.startStream();
    sendHeadandTail(F("TmplMsg"), _HEAD);
    addHtmlError(error);
    TXBuffer.endStream();
  }

  checkRAM(F("handle_rules_backup"));
}

void handle_rules_delete() {
  if (!isLoggedIn() || !Settings.UseRules) { return; }

  if (!clientIPallowed()) { return; }

  if (Settings.OldRulesEngine())
  {
    Goto_Rules_Root();
    return;
  }
  checkRAM(F("handle_rules_delete"));
  String fileName = WebServer.arg(F("fileName"));
  fileName = fileName.substring(0, fileName.length() - 4);
  bool removed = false;
  #ifdef WEBSERVER_RULES_DEBUG
  Serial.println(F("handle_rules_delete"));
  Serial.print(F("File name: "));
  Serial.println(fileName);
  #endif // ifdef WEBSERVER_RULES_DEBUG

  if (fileName.length() > 0)
  {
    removed = SPIFFS.remove(fileName);
  }

  if (removed)
  {
    WebServer.sendHeader(F("Location"), F("/rules"), true);
    WebServer.send(302, F("text/plain"), F(""));
  }
  else
  {
    String error = String(F("Delete rule Invalid path: ")) + fileName;
    addLog(LOG_LEVEL_ERROR, error);
    TXBuffer.startStream();
    sendHeadandTail(F("TmplMsg"), _HEAD);
    addHtmlError(error);
    sendHeadandTail(F("TmplMsg"), _TAIL);
    TXBuffer.endStream();
  }
  checkRAM(F("handle_rules_delete"));
}

bool handle_rules_edit(const String& originalUri)
{
  return handle_rules_edit(originalUri, false);
}

bool handle_rules_edit(String originalUri, bool isAddNew) {
  // originalUri is passed via deepcopy, since it will be converted to lower case.
  if (!isLoggedIn() || !Settings.UseRules) { return false; }
  originalUri.toLowerCase();
  checkRAM(F("handle_rules"));
  bool handle = false;

  #ifdef WEBSERVER_RULES_DEBUG
  Serial.println(originalUri);
  Serial.println(F("handle_rules_edit"));
  #endif // ifdef WEBSERVER_RULES_DEBUG

  if (isAddNew || (originalUri.startsWith(F("/rules/"))
                   && originalUri.endsWith(F(".txt")))) {
    if (Settings.OldRulesEngine())
    {
      Goto_Rules_Root();
      return true;
    }

    String eventName;
    String fileName;
    bool   isOverwrite = false;
    bool   isNew       = false;
    String rules;
    String error;


    if (isAddNew)
    {
      eventName = WebServer.arg(F("eventName"));
      fileName += EventToFileName(eventName);
    }
    else
    {
        #if defined(ESP8266)
      fileName = F("rules/");
        #endif // if defined(ESP8266)
        #if defined(ESP32)
      fileName = F("/rules/");
        #endif // if defined(ESP32)
      fileName += originalUri.substring(7, originalUri.length() - 4);
      eventName = FileNameToEvent(fileName);
    }
      #ifdef WEBSERVER_RULES_DEBUG
    Serial.print(F("File name: "));
    Serial.println(fileName);
      #endif // ifdef WEBSERVER_RULES_DEBUG
    bool isEdit = SPIFFS.exists(fileName);

    if (WebServer.args() > 0)
    {
      rules = WebServer.arg(F("rules"));
      isNew = WebServer.arg(F("IsNew")) == F("yes");

      // Overwrite verification
      if (isEdit && isNew) {
        error = String(F("There is another rule with the same name."))
                + fileName;
        addLog(LOG_LEVEL_ERROR, error);
        isAddNew    = true;
        isOverwrite = true;
      }

      // Check rules size
      else if (rules.length() > RULES_MAX_SIZE)
      {
        error = String(F("Data was not saved, exceeds web editor limit! "))
                + fileName;
        addLog(LOG_LEVEL_ERROR, error);
      }

      // Passed all checks, write file
      else
      {
        fs::File f = tryOpenFile(fileName, "w");

        if (f)
        {
          addLog(LOG_LEVEL_INFO, String(F(" Write to file: ")) + fileName);
          f.print(rules);
          f.close();
        }

        if (isAddNew) {
          WebServer.sendHeader(F("Location"), F("/rules"), true);
          WebServer.send(302, F("text/plain"), F(""));
          return true;
        }
      }
    }
    navMenuIndex = 5;
    TXBuffer.startStream();
    sendHeadandTail(F("TmplStd"));

    if (error.length() > 0) {
      addHtmlError(error);
    }
    TXBuffer += F("<form name = 'editRule' method = 'post'><table class='normal'><TR><TH align='left' colspan='2'>Edit Rule");

    // hidden field to check Overwrite
    TXBuffer += F("<input type='hidden' id='IsNew' name='IsNew' value='");
    TXBuffer += isAddNew
                ? F("yes")
                : F("no");
    TXBuffer += F("'>");

    bool isReadOnly = !isOverwrite && ((isEdit && !isAddNew && !isNew) || (isAddNew && isNew));
      #ifdef WEBSERVER_RULES_DEBUG
    Serial.print(F("Is Overwrite: "));
    Serial.println(isOverwrite);
    Serial.print(F("Is edit: "));
    Serial.println(isEdit);
    Serial.print(F("Is addnew: "));
    Serial.println(isAddNew);
    Serial.print(F("Is New: "));
    Serial.println(isNew);
    Serial.print(F("Is Read Only: "));
    Serial.println(isReadOnly);
      #endif // ifdef WEBSERVER_RULES_DEBUG

    addFormTextBox(F("Event name")            // Label
                   , F("eventName")           // field name
                   , eventName                // field value
                   , RULE_MAX_FILENAME_LENGTH // max length
                   , isReadOnly               // is readonly
                   , isAddNew                 // required
                   , F("[A-Za-z]+#.+")        // validation pattern
                   );
    addHelpButton(F("Tutorial_Rules"));

    // load form data from flash
    TXBuffer += F("<TR><TD colspan='2'>");
    int size = 0;

    if (!isOverwrite)
    {
      rules = "";
      fs::File f = tryOpenFile(fileName, "r");

      if (f)
      {
        size = f.size();

        if (size < RULES_MAX_SIZE)
        {
          rules.reserve(size);

          while (f.available())
          {
            rules += (char)f.read();
          }
        }
        f.close();
      }
    }

    if (size > RULES_MAX_SIZE) {
      TXBuffer += F("<span style=\"color:red\">Filesize exceeds web editor limit!</span>");
    }
    else
    {
      TXBuffer += F("<textarea name='rules' rows='30' wrap='off'>");
      String c(rules);
      htmlEscape(c);
      TXBuffer += c;
      TXBuffer += F("</textarea>");
    }

    TXBuffer += F("<TR><TD colspan='2'>");

    html_TR_TD(); TXBuffer += F("Current size: ");
    TXBuffer               += size;
    TXBuffer               += F(" characters (Max ");
    TXBuffer               += RULES_MAX_SIZE;
    TXBuffer               += F(")");
    addFormSeparator(2);
    html_TR_TD();
    addSubmitButton();

    TXBuffer += F("</table></form>");

    sendHeadandTail(F("TmplStd"), true);
    TXBuffer.endStream();
  }

  checkRAM(F("handle_rules"));
  return handle;
}

bool Rule_Download(const String& path)
{
  #ifdef WEBSERVER_RULES_DEBUG
  Serial.print(F("Rule_Download path: "));
  Serial.println(path);
  #endif // ifdef WEBSERVER_RULES_DEBUG
  fs::File dataFile = tryOpenFile(path, "r");

  if (!dataFile)
  {
    addLog(LOG_LEVEL_ERROR, String(F("Invalid path: ")) + path);
    return false;
  }
  String filename = path + String(F(".txt"));
  filename.replace(RULE_FILE_SEPARAROR, '_');
  String str = String(F("attachment; filename=")) + filename;
  WebServer.sendHeader(F("Content-Disposition"), str);
  WebServer.sendHeader(F("Cache-Control"),       F("max-age=3600, public"));
  WebServer.sendHeader(F("Vary"),                "*");
  WebServer.sendHeader(F("ETag"),                F("\"2.0.0\""));

  WebServer.streamFile(dataFile, F("application/octet-stream"));
  dataFile.close();
  return true;
}

void Goto_Rules_Root() {
  WebServer.sendHeader(F("Location"), F("/rules"), true);
  WebServer.send(302, F("text/plain"), F(""));
}

bool EnumerateFileAndDirectory(String          & rootPath
                               , int             skip
                               , HandlerFileInfo handler)
{
  bool hasMore = false;
  int  count   = 0;
  bool next    = true;

  #ifdef ESP8266
  fs::Dir dir = SPIFFS.openDir(rootPath);
  Serial.print(F("Enumerate files of "));
  Serial.println(rootPath);

  while (next && dir.next()) {
    // Skip files
    if (count++ < skip) {
      continue;
    }

    // Workaround for skipped unwanted files
    if (!dir.fileName().startsWith(rootPath + "/")) {
      continue;
    }
    fileInfo fi;
    fi.Name = dir.fileName() /*.substring(rootPath.length())*/;
    fs::File f = dir.openFile("r");
    fi.Size = f.size();
    f.close();
    next = handler(fi);
  }
  hasMore = dir.next();
  #endif // ifdef ESP8266
  #ifdef ESP32
  File root = SPIFFS.open(rootPath);

  if (root)
  {
    File file = root.openNextFile();

    while (next && file) {
      if (count > skip) {
        fileInfo fi;
        fi.Name        = file.name();
        fi.Size        = file.size();
        fi.isDirectory = file.isDirectory();
        next           = handler(fi);
      }

      if (!file.isDirectory()) {
        ++count;
      }
      file = root.openNextFile();
    }
  }
  else
  {
    addLog(LOG_LEVEL_ERROR, F("Invalid root."));
  }
  #endif // ifdef ESP32
  return hasMore;
}
