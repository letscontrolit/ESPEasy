#include "../WebServer/FileList.h"

#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"

#include "../ESPEasyCore/ESPEasyRules.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Numerical.h"

#include "../../ESPEasy_common.h"



#ifdef USES_C016
#include "../Globals/C016_ControllerCache.h"
#endif

#if FEATURE_SD
#include <SD.h>
#endif // if FEATURE_SD


#define FILES_PER_PAGE   50

#ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface file list
// ********************************************************************************
void handle_filelist_json() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_filelist"));
  #endif

  if (!clientIPallowed()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startJsonStream();

  String fdelete = webArg(F("delete"));

  if (tryDeleteFile(fdelete)) {
    # if defined(ESP32)

    // flashCount();
    # endif // if defined(ESP32)
    # if defined(ESP8266)
    checkRuleSets();
    # endif // if defined(ESP8266)
  }

  int startIdx       = 0;

  String fstart = webArg(F("start"));

  if (fstart.length() > 0)
  {
    validIntFromString(fstart, startIdx);
  }
  int endIdx = startIdx + FILES_PER_PAGE - 1;

  addHtml('[', '{');
  bool firstentry = true;
  # if defined(ESP32)
  fs::File root  = ESPEASY_FS.open("/");
  fs::File file  = root.openNextFile();
  int  count = -1;

  while (file and count < endIdx)
  {
    if (!file.isDirectory()) {
      ++count;

      if (count >= startIdx)
      {
        if (firstentry) {
          firstentry = false;
        } else {
          addHtml(',', '{');
        }
        stream_next_json_object_value(F("fileName"), String(file.name()));
        stream_next_json_object_value(F("index"),    startIdx);
        stream_last_json_object_value(F("size"), file.size());
      }
    }
    file = root.openNextFile();
  }
  # endif // if defined(ESP32)
  # if defined(ESP8266)
  fs::Dir dir = ESPEASY_FS.openDir("");

  int count = -1;

  while (dir.next())
  {
    ++count;

    if (count < startIdx)
    {
      continue;
    }

    if (firstentry) {
      firstentry = false;
    } else {
      addHtml(',', '{');
    }

    stream_next_json_object_value(F("fileName"), String(dir.fileName()));

    fs::File f = dir.openFile("r");

    if (f) {
      stream_next_json_object_value(F("size"), f.size());
      f.close();
    }

    stream_last_json_object_value(F("index"), startIdx);

    if (count >= endIdx)
    {
      break;
    }
  }

  if (firstentry) {
    addHtml('}');
  }

  # endif // if defined(ESP8266)
  addHtml(']');
  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI

#ifdef WEBSERVER_FILELIST
void handle_filelist() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_filelist"));
  #endif

  if (!clientIPallowed()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  String fdelete = webArg(F("delete"));

  if (tryDeleteFile(fdelete))
  {
    checkRuleSets();
  }
  # ifdef USES_C016

  if (hasArg(F("delcache"))) {
    addLog(LOG_LEVEL_INFO, F("RTC  : delcache"));
    C016_deleteAllCacheBlocks();

    while (GarbageCollection()) {
      delay(1);
    }
  }
  # endif // ifdef USES_C016
  int startIdx       = 0;
  String fstart      = webArg(F("start"));

  if (fstart.length() > 0)
  {
    validIntFromString(fstart, startIdx);
  }
  int endIdx = startIdx + FILES_PER_PAGE - 1;
  html_table_class_multirow();
  html_table_header(F(""),        50);
  html_table_header(F("Filename"));
  html_table_header(F("Size"), 80);
  int count = -1;

  bool moreFilesPresent  = false;
#if FEATURE_RTC_CACHE_STORAGE
  bool cacheFilesPresent = false;
#endif

# if defined(ESP8266)

  fs::Dir dir = ESPEASY_FS.openDir("");

  while (dir.next() && count < endIdx)
  {
    ++count;

    if (count >= startIdx)
    {
      int filesize = -1;
      fs::File f   = dir.openFile("r");

      if (f) {
        filesize = f.size();
      }
#if FEATURE_RTC_CACHE_STORAGE
      if (!cacheFilesPresent && (getCacheFileCountFromFilename(dir.fileName()) != -1))
      {
        cacheFilesPresent = true;
      }
#endif
      handle_filelist_add_file(dir.fileName(), filesize, startIdx);
    }
  }
  moreFilesPresent = dir.next();
# endif // if defined(ESP8266)
# if defined(ESP32)
  fs::File root = ESPEASY_FS.open("/");
  fs::File file = root.openNextFile();

  while (file && count < endIdx)
  {
    if (!file.isDirectory()) {
      ++count;

      if (count >= startIdx)
      {
#if FEATURE_RTC_CACHE_STORAGE
        if (!cacheFilesPresent && (getCacheFileCountFromFilename(file.name()) != -1))
        {
          cacheFilesPresent = true;
        }
#endif
        handle_filelist_add_file(file.name(), file.size(), startIdx);
      }
    }
    file = root.openNextFile();
  }
  moreFilesPresent = file;
# endif // if defined(ESP32)

  int start_prev = -1;

  if (startIdx > 0)
  {
    start_prev = startIdx < FILES_PER_PAGE ? 0 : startIdx - FILES_PER_PAGE;
  }
  int start_next = -1;

  if ((count >= endIdx) && moreFilesPresent) {
    start_next = endIdx + 1;
  }
#if FEATURE_RTC_CACHE_STORAGE
  handle_filelist_buttons(start_prev, start_next, cacheFilesPresent);
#else
  handle_filelist_buttons(start_prev, start_next, false);
#endif
}

void handle_filelist_add_file(const String& filename, int filesize, int startIdx) {
  html_TR_TD();

  if (!isProtectedFileType(filename))
  {
    html_add_button_prefix();
    addHtml(F("filelist?delete="));
    addHtml(filename);

    if (startIdx > 0)
    {
      addHtml(F("&start="));
      addHtmlInt(startIdx);
    }
    addHtml(F("'>Del</a>"));
  }
  {
    addHtml(F("<TD><a href=\""));
    addHtml(filename);
    addHtml('"', '>');
    addHtml(filename);
    addHtml(F("</a><TD>"));

    if (filesize >= 0) {
      addHtmlInt(filesize);
    }
  }
}

void handle_filelist_buttons(int start_prev, int start_next, bool cacheFilesPresent) {
  html_end_table();
  html_end_form();
  html_BR();
  addButton(F("/upload"), F("Upload"));

  if (start_prev >= 0)
  {
    html_add_button_prefix();
    addHtml(F("/filelist?start="));
    addHtmlInt(start_prev);
    addHtml(F("'>Previous</a>"));
  }

  if (start_next >= 0)
  {
    html_add_button_prefix();
    addHtml(F("/filelist?start="));
    addHtmlInt(start_next);
    addHtml(F("'>Next</a>"));
  }
#if FEATURE_RTC_CACHE_STORAGE
  if (cacheFilesPresent) {
    html_add_button_prefix(F("red"), true);
    addHtml(F("filelist?delcache=1'>Delete Cache Files</a>"));
  }
#endif
  addHtml(F("<BR><BR>"));
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // ifdef WEBSERVER_FILELIST

// ********************************************************************************
// Web Interface SD card file and directory list
// ********************************************************************************
#if FEATURE_SD
void handle_SDfilelist() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_SDfilelist"));
  #endif

  if (!clientIPallowed()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);


  String fdelete;
  String ddelete;
  String change_to_dir;
  String current_dir;
  String parent_dir;

  for (uint8_t i = 0; i < web_server.args(); i++) {
    // FIXME TD-er: This only checks for arguments in the URL, not in POST args
    // It also takes only the last matching argument.
    if (equals(web_server.argName(i), F("delete")))
    {
      fdelete = webArg(i);
    }

    if (equals(web_server.argName(i), F("deletedir")))
    {
      ddelete = webArg(i);
    }

    if (equals(web_server.argName(i), F("chgto")))
    {
      change_to_dir = webArg(i);
    }
  }

  if (fdelete.length() > 0)
  {
    SD.remove((char *)fdelete.c_str());
  }

  if (ddelete.length() > 0)
  {
    SD.rmdir((char *)ddelete.c_str());
  }

  if (change_to_dir.length() > 0)
  {
    current_dir = change_to_dir;
  }
  else
  {
    current_dir = "/";
  }

  fs::File root = SD.open(current_dir.c_str());
  root.rewindDirectory();
  fs::File entry = root.openNextFile();
  parent_dir = current_dir;

  if (!current_dir.equals("/"))
  {
    /* calculate the position to remove
       /
       / current_dir = /dir1/dir2/   =>   parent_dir = /dir1/
       /                     ^ position to remove, second last index of "/" + 1
       /
       / current_dir = /dir1/   =>   parent_dir = /
       /                ^ position to remove, second last index of "/" + 1
     */
    parent_dir.remove(parent_dir.lastIndexOf("/", parent_dir.lastIndexOf("/") - 1) + 1);
  }


  addFormSubHeader(String(F("SD Card: ")) + current_dir);
  html_BR();
  html_table_class_multirow();
  html_table_header(F(""), 50);
  html_table_header(F("Name"));
  html_table_header(F("Size"));
  html_TR_TD();
  {
    addHtml(F("<TD><a href=\"SDfilelist?chgto="));
    addHtml(parent_dir);
    addHtml(F("\">..</a><TD>"));
  }

  while (entry)
  {
    html_TR_TD();
    // size_t entrynameLength = strlen(entry.name());
    if (entry.isDirectory())
    {
      char SDcardChildDir[80];

      // take a look in the directory for entries
      String child_dir = current_dir + entry.name();
      child_dir.toCharArray(SDcardChildDir, child_dir.length() + 1);
      fs::File child         = SD.open(SDcardChildDir);
      fs::File dir_has_entry = child.openNextFile();

      // when the directory is empty, display the button to delete them
      if (!dir_has_entry)
      {
        addHtml(F("<a class='button link' onclick=\"return confirm('Delete this directory?')\" href=\"SDfilelist?deletedir="));
        addHtml(current_dir);
        addHtml(entry.name());
        addHtml('/');
        addHtml(F("&chgto="));
        addHtml(current_dir);
        addHtml(F("\">Del</a>"));
      }
      {
        addHtml(F("<TD><a href=\"SDfilelist?chgto="));
        addHtml(current_dir);
        addHtml(entry.name());
        addHtml('/');
        addHtml('"', '>');
        addHtml(entry.name());
        addHtml(F("</a><TD>dir"));
      }
      dir_has_entry.close();
    }
    else
    {

      if (isProtectedFileType(String(entry.name())))
      {
        addHtml(F("<a class='button link' onclick=\"return confirm('Delete this file?')\" href=\"SDfilelist?delete="));
        addHtml(current_dir);
        addHtml(entry.name());
        addHtml(F("&chgto="));
        addHtml(current_dir);
        addHtml(F("\">Del</a>"));
      }
      {
        // FIXME TD-er: There's a lot of code duplication here.
        addHtml(F("<TD><a href=\""));
        addHtml(current_dir);
        addHtml(entry.name());
        addHtml('"', '>');
        addHtml(entry.name());
        addHtml(F("</a><TD>"));
        addHtml(entry.size());
      }
    }
    entry.close();
    entry = root.openNextFile();
  }
  root.close();
  html_end_table();
  html_end_form();

  // addHtml(F("<BR><a class='button link' href=\"/upload\">Upload</a>"));
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // if FEATURE_SD
