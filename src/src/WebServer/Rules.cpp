// #define WEBSERVER_RULES_DEBUG

#include "../WebServer/Rules.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"

#include "../ESPEasyCore/ESPEasyRules.h"

#include "../Globals/Settings.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"
#include "../Static/WebStaticData.h"

#include <FS.h>

#ifdef WEBSERVER_RULES

// ********************************************************************************
// Web Interface rules page
// ********************************************************************************
void handle_rules() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_rules"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn() || !Settings.UseRules) { return; }
  navMenuIndex = MENU_INDEX_RULES;
  const byte rulesSet = getFormItemInt(F("set"), 1);

  # if defined(ESP8266)
  String fileName = F("rules");
  # endif // if defined(ESP8266)
  # if defined(ESP32)
  String fileName = F("/rules");
  # endif // if defined(ESP32)
  fileName += rulesSet;
  fileName += F(".txt");

  String error;

  // Make sure file exists
  if (!fileExists(fileName))
  {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("Rules : Create new file: ");
      log += fileName;
      addLog(LOG_LEVEL_INFO, log);
    }
    fs::File f = tryOpenFile(fileName, "w");

    if (f) { f.close(); }
  }

  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();
  addHtmlError(error);

  html_table_class_normal();
  html_TR();
  html_table_header(F("Rules"));

  html_TR_TD();
  addHtml(F("<form name = 'frmselect'>"));
  {
    // Place combo box in its own scope to release these arrays as soon as possible
    byte   choice = rulesSet;
    String options[RULESETS_MAX];
    int    optionValues[RULESETS_MAX];

    for (byte x = 0; x < RULESETS_MAX; x++)
    {
      options[x]      = F("Rules Set ");
      options[x]     += x + 1;
      optionValues[x] = x + 1;
    }

    addSelector(F("set"), RULESETS_MAX, options, optionValues, NULL, choice, true, true);
    addHelpButton(F("Tutorial_Rules"));
    addRTDHelpButton(F("Rules/Rules.html"));
  }

  html_TR_TD();
  Rule_showRuleTextArea(fileName);

  html_TR_TD();
  html_end_form();
  addHtml(F("<button id='save_button' class='button' onClick='saveRulesFile()'>Save</button>"));
  addHtmlDiv(EMPTY_STRING, F("Saved!"), F("toastmessage"));

  addButton(fileName, F("Download to file"));
  html_end_table();

  serve_JS(JSfiles_e::SaveRulesFile);

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
  # ifdef WEBSERVER_NEW_RULES
  #  ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_rules"));
  #  endif // ifndef BUILD_NO_RAM_TRACKER
  navMenuIndex = 5;
  TXBuffer.startStream();
  sendHeadandTail(F("TmplStd"), _HEAD);

  // define macro
  #  if defined(ESP8266)
  String rootPath = F("rules");
  #  endif // if defined(ESP8266)
  #  if defined(ESP32)

  String rootPath = F("/rules");
  #  endif // if defined(ESP32)

  // Pagionation of rules list
  const int rulesListPageSize = 25;
  int startIdx                = 0;

  const String fstart = webArg(F("start"));

  if (fstart.length() > 0)
  {
    validIntFromString(fstart, startIdx);
  }
  int endIdx = startIdx + rulesListPageSize - 1;

  // Build table header
  html_table_class_multirow();
  html_TR();
  html_table_header(F("Event Name"));
  html_table_header(F("Filename"));
  html_table_header(F("Size"));
  addHtml(F("<TH>Actions"));
  addSaveButton(F("/rules/backup"), F("Backup"));
  addHtml(F("</TH></TR>"));

  // class StreamingBuffer buffer = TXBuffer;

  // Build table detail
  int count = -1;
  HandlerFileInfo renderDetail = [/*&buffer,*/ &count, endIdx](fileInfo fi)
                                 {
#  ifdef WEBSERVER_RULES_DEBUG
                                   Serial.print(F("Start generation of: "));
                                   Serial.println(fi.Name);
#  endif // ifdef WEBSERVER_RULES_DEBUG

                                   if (fi.isDirectory)
                                   {
                                     html_TR_TD();
                                   }
                                   else
                                   {
                                     count++;
                                     addHtml(F("<TR><TD style='text-align:right'>"));
                                   }

                                   // Event Name
                                   addHtml(FileNameToEvent(fi.Name));

                                   if (fi.isDirectory)
                                   {
                                     addHtml(F("</TD><TD></TD><TD></TD><TD>"));
                                     addSaveButton(String(F("/rules/backup?directory=")) + URLEncode(fi.Name.c_str())
                                                   , F("Backup")
                                                   );
                                   }
                                   else
                                   {
                                     String encodedPath =  URLEncode((fi.Name + F(".txt")).c_str());

                                     // File Name
                                     {
                                       String html;
                                       html.reserve(128);

                                       html += F("</TD><TD><a href='");
                                       html += fi.Name;
                                       html += F(".txt");
                                       html += "'>";
                                       html += fi.Name;
                                       html += F(".txt");
                                       html += F("</a></TD>");

                                       // File size
                                       html += F("<TD>");
                                       html += fi.Size;
                                       html += F("</TD>");
                                       addHtml(html);
                                     }

                                     // Actions
                                     html_TD();
                                     addSaveButton(String(F("/rules/backup?fileName=")) + encodedPath
                                                   , F("Backup")
                                                   );

                                     addDeleteButton(String(F("/rules/delete?fileName=")) + encodedPath
                                                     , F("Delete")
                                                     );
                                   }
                                   addHtml(F("</TD></TR>"));
#  ifdef WEBSERVER_RULES_DEBUG
                                   Serial.print(F("End generation of: "));
                                   Serial.println(fi.Name);
#  endif // ifdef WEBSERVER_RULES_DEBUG

                                   return count < endIdx;
                                 };


  bool hasMore = EnumerateFileAndDirectory(rootPath
                                           , startIdx
                                           , renderDetail);
  html_TR_TD();
  addButton(F("/rules/add"), F("Add"));
  addHtml(F("</TD><TD></TD><TD></TD><TD></TD></TR>"));
  addHtml(F("</table>"));

  if (startIdx > 0)
  {
    int showIdx = startIdx - rulesListPageSize;

    if (showIdx < 0) { showIdx = 0; }
    addButton(String(F("/rules?start=")) + String(showIdx)
              , F("Previous"));
  }

  if (hasMore && (count >= endIdx))
  {
    addButton(String(F("/rules?start=")) + String(endIdx + 1)
              , F("Next"));
  }

  // TXBuffer += F("<BR><BR>");
  sendHeadandTail(F("TmplStd"), _TAIL);
  TXBuffer.endStream();
  #  ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_rules"));
  #  endif // ifndef BUILD_NO_RAM_TRACKER
  # endif  // WEBSERVER_NEW_RULES
}

void handle_rules_backup() {
  if (Settings.OldRulesEngine())
  {
    Goto_Rules_Root();
    return;
  }
  # ifdef WEBSERVER_NEW_RULES
  #  ifdef WEBSERVER_RULES_DEBUG
  Serial.println(F("handle rules backup"));
  #  endif // ifdef WEBSERVER_RULES_DEBUG

  if (!isLoggedIn() || !Settings.UseRules) { return; }

  if (!clientIPallowed()) { return; }
  #  ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_rules_backup"));
  #  endif // ifndef BUILD_NO_RAM_TRACKER
  String directory = webArg(F("directory"));
  String fileName  = webArg(F("fileName"));
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
  #  ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_rules_backup"));
  #  endif // ifndef BUILD_NO_RAM_TRACKER
  # endif  // WEBSERVER_NEW_RULES
}

void handle_rules_delete() {
  if (!isLoggedIn() || !Settings.UseRules) { return; }

  if (!clientIPallowed()) { return; }

  if (Settings.OldRulesEngine())
  {
    Goto_Rules_Root();
    return;
  }
  # ifdef WEBSERVER_NEW_RULES
  #  ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_rules_delete"));
  #  endif // ifndef BUILD_NO_RAM_TRACKER
  String fileName = webArg(F("fileName"));
  fileName = fileName.substring(0, fileName.length() - 4);
  bool removed = false;
  #  ifdef WEBSERVER_RULES_DEBUG
  Serial.println(F("handle_rules_delete"));
  Serial.print(F("File name: "));
  Serial.println(fileName);
  #  endif // ifdef WEBSERVER_RULES_DEBUG

  if (fileName.length() > 0)
  {
    removed = tryDeleteFile(fileName);
  }

  if (removed)
  {
    web_server.sendHeader(F("Location"), F("/rules"), true);
    web_server.send(302, F("text/plain"), EMPTY_STRING);
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
  #  ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_rules_delete"));
  #  endif // ifndef BUILD_NO_RAM_TRACKER
  # endif  // WEBSERVER_NEW_RULES
}

bool handle_rules_edit(const String& originalUri)
{
  return handle_rules_edit(originalUri, false);
}

bool handle_rules_edit(String originalUri, bool isAddNew) {
  // originalUri is passed via deepcopy, since it will be converted to lower case.
  originalUri.toLowerCase();
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_rules_edit"));
  # endif // ifndef BUILD_NO_RAM_TRACKER
  bool handle = false;

  # ifdef WEBSERVER_RULES_DEBUG
  Serial.println(originalUri);
  Serial.println(F("handle_rules_edit"));
  # endif // ifdef WEBSERVER_RULES_DEBUG

  if (isAddNew || (originalUri.startsWith(F("/rules/"))
                   && originalUri.endsWith(F(".txt")))) {
    if (!isLoggedIn() || !Settings.UseRules) { return false; }
    if (Settings.OldRulesEngine())
    {
      Goto_Rules_Root();
      return true;
    }
    # ifdef WEBSERVER_NEW_RULES

    String eventName;
    String fileName;
    bool   isOverwrite = false;
    bool   isNew       = false;
    String error;


    if (isAddNew)
    {
      eventName = webArg(F("eventName"));
      fileName += EventToFileName(eventName);
    }
    else
    {
        #  if defined(ESP8266)
      fileName = F("rules/");
        #  endif // if defined(ESP8266)
        #  if defined(ESP32)
      fileName = F("/rules/");
        #  endif // if defined(ESP32)
      fileName += originalUri.substring(7, originalUri.length() - 4);
      eventName = FileNameToEvent(fileName);
    }
      #  ifdef WEBSERVER_RULES_DEBUG
    Serial.print(F("File name: "));
    Serial.println(fileName);
      #  endif // ifdef WEBSERVER_RULES_DEBUG
    bool isEdit = fileExists(fileName);

    if (web_server.args() > 0)
    {
      const String& rules = webArg(F("rules"));
      isNew = webArg(F("IsNew")) == F("yes");

      // Overwrite verification
      if (isEdit && isNew) {
        error = String(F("There is another rule with the same name: "))
                + fileName;
        addLog(LOG_LEVEL_ERROR, error);
        isAddNew    = true;
        isOverwrite = true;
      }
      else if (!web_server.hasArg(F("rules")))
      {
        error = F("Data was not saved, rules argument missing or corrupted");
        addLog(LOG_LEVEL_ERROR, error);
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
          web_server.sendHeader(F("Location"), F("/rules"), true);
          web_server.send(302, F("text/plain"), EMPTY_STRING);
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
    addHtml(F("<form name = 'editRule' method = 'post'><table class='normal'><TR><TH align='left' colspan='2'>Edit Rule"));

    // hidden field to check Overwrite
    addHtml(F("<input "));
    addHtmlAttribute(F("type"),  F("hidden"));
    addHtmlAttribute(F("id"),    F("IsNew"));
    addHtmlAttribute(F("name"),  F("IsNew"));
    addHtmlAttribute(F("value"), isAddNew ? F("yes") : F("no"));
    addHtml('>');

    bool isReadOnly = !isOverwrite && ((isEdit && !isAddNew && !isNew) || (isAddNew && isNew));
      #  ifdef WEBSERVER_RULES_DEBUG
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
      #  endif // ifdef WEBSERVER_RULES_DEBUG

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
    addHtml(F("<TR><TD colspan='2'>"));

    Rule_showRuleTextArea(fileName);

    addFormSeparator(2);
    html_TR_TD();
    addSubmitButton();

    addHtml(F("</table></form>"));

    sendHeadandTail(F("TmplStd"), true);
    TXBuffer.endStream();
    # endif // WEBSERVER_NEW_RULES
  }
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_rules_edit2"));
  # endif // ifndef BUILD_NO_RAM_TRACKER
  return handle;
}

void Rule_showRuleTextArea(const String& fileName) {
  // Read rules from file and stream directly into the textarea

  size_t size = 0;

  addHtml(F("<textarea id='rules' name='rules' rows='30' wrap='off'>"));
  size = streamFile_htmlEscape(fileName);
  addHtml(F("</textarea>"));

  html_TR_TD();
  {
    String html;
    html.reserve(64);

    html += F("Current size: <span id='size'>");
    html += size;
    html += F("</span> characters (Max ");
    html += RULES_MAX_SIZE;
    html += F(")");
    addHtml(html);
  }

  if (size > RULES_MAX_SIZE) {
    addHtml(F("<span style=\"color:red\">Filesize exceeds web editor limit!</span>"));
  }
}

bool Rule_Download(const String& path)
{
  # ifdef WEBSERVER_RULES_DEBUG
  Serial.print(F("Rule_Download path: "));
  Serial.println(path);
  # endif // ifdef WEBSERVER_RULES_DEBUG
  fs::File dataFile = tryOpenFile(path, "r");

  if (!dataFile)
  {
    addLog(LOG_LEVEL_ERROR, String(F("Invalid path: ")) + path);
    return false;
  }
  String filename = path + String(F(".txt"));
  filename.replace(RULE_FILE_SEPARAROR, '_');
  String str = String(F("attachment; filename=")) + filename;
  web_server.sendHeader(F("Content-Disposition"), str);
  web_server.sendHeader(F("Cache-Control"),       F("max-age=3600, public"));
  web_server.sendHeader(F("Vary"),                "*");
  web_server.sendHeader(F("ETag"),                F("\"2.0.0\""));

  web_server.streamFile(dataFile, F("application/octet-stream"));
  dataFile.close();
  return true;
}

void Goto_Rules_Root() {
  web_server.sendHeader(F("Location"), F("/rules"), true);
  web_server.send(302, F("text/plain"), EMPTY_STRING);
}

bool EnumerateFileAndDirectory(String          & rootPath
                               , int             skip
                               , HandlerFileInfo handler)
{
  bool hasMore = false;
  int  count   = 0;
  bool next    = true;

  # ifdef ESP8266
  fs::Dir dir = ESPEASY_FS.openDir(rootPath);
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
  # endif // ifdef ESP8266
  # ifdef ESP32
  File root = ESPEASY_FS.open(rootPath);

  if (root)
  {
    File file = root.openNextFile();

    while (next && file) {
      if (count >= skip) {
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
    hasMore = file;
  }
  else
  {
    addLog(LOG_LEVEL_ERROR, F("Invalid root."));
  }
  # endif // ifdef ESP32
  return hasMore;
}

#endif // ifdef WEBSERVER_RULES
