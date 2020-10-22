#include "../WebServer/SettingsArchive.h"

#ifdef USE_SETTINGS_ARCHIVE

#include "../WebServer/WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"

#include "../Globals/ResetFactoryDefaultPref.h"
#include "../Helpers/ESPEasy_FactoryDefault.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"


// ********************************************************************************
// Web Interface to manage archived settings
// ********************************************************************************
void handle_settingsarchive() {
  checkRAM(F("handle_settingsarchive"));

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);
  html_add_form();
  html_table_class_normal();
  html_TR();
  addFormHeader(F("Settings Archive"));

  if (web_server.hasArg(F("savepref")) || web_server.hasArg(F("download"))) {
    // User choose a pre-defined config and wants to save it as the new default.
    storeDownloadFiletypeCheckbox(FileType::CONFIG_DAT);
    storeDownloadFiletypeCheckbox(FileType::SECURITY_DAT);
    storeDownloadFiletypeCheckbox(FileType::NOTIFICATION_DAT);

    for (int i = 0; i < 4; ++i) {
      storeDownloadFiletypeCheckbox(FileType::RULES_TXT, i);
    }

    ResetFactoryDefaultPreference.deleteFirst(isFormItemChecked("del"));

    applyFactoryDefaultPref();
    addHtmlError(SaveSettings());
  }

  bool showOptions = true;

  if (web_server.hasArg(F("download"))) {
    // Try downloading files.
    String url  = web_server.arg(F("url"));
    String user = web_server.arg(F("user"));
    String pass = web_server.arg(F("pass"));

    addTableSeparator(F("Download result"), 2, 3);
    bool somethingDownloaded = false;

    if (tryDownloadFileType(url, user, pass, FileType::CONFIG_DAT)) { somethingDownloaded = true; }

    if (tryDownloadFileType(url, user, pass, FileType::SECURITY_DAT)) { somethingDownloaded = true; }

    if (tryDownloadFileType(url, user, pass, FileType::NOTIFICATION_DAT)) { somethingDownloaded = true; }

    for (int i = 0; i < 4; ++i) {
      if (tryDownloadFileType(url, user, pass, FileType::RULES_TXT, i)) { somethingDownloaded = true; }
    }

    if (somethingDownloaded) {
      showOptions = false;
      html_TR_TD();
      html_TD();
      addSubmitButton(F("Reboot"), F("reboot"), F("red"));
      addFormNote(F("If settings files are updated you MUST reboot first!"));
    }
  } else if (web_server.hasArg(F("reboot"))) {
    showOptions = false;
    reboot(ESPEasy_Scheduler::IntendedRebootReason_e::RestoreSettings);
  }

  if (showOptions) {
    // Nothing chosen yet, show options.

    addTableSeparator(F("Files to Download"), 2, 3);
    addDownloadFiletypeCheckbox(FileType::CONFIG_DAT);
    addDownloadFiletypeCheckbox(FileType::SECURITY_DAT);
    addDownloadFiletypeCheckbox(FileType::NOTIFICATION_DAT);

    for (int i = 0; i < 4; ++i) {
      addDownloadFiletypeCheckbox(FileType::RULES_TXT, i);
    }

    addTableSeparator(F("Download Settings"), 2, 3);

    addRowLabel(F("Delete First"));
    addCheckBox("del", ResetFactoryDefaultPreference.deleteFirst());
    addFormNote(F("Needed on filesystem with not enough free space. Use with care!"));


    html_TR_TD();
    html_TD();
    addSubmitButton(F("Save Preferences"), F("savepref"));

    addTableSeparator(F("Archive Location"), 2, 3);

    addFormTextBox(F("URL with settings"), F("url"), web_server.arg(F("url")), 256);
    addFormNote(F("Only HTTP supported. Do not include filename"));
    addFormTextBox(F("User"), F("user"), web_server.arg(F("user")), 64);
    addFormPasswordBox(F("Pass"), F("pass"), web_server.arg(F("pass")), 64);
    addFormNote(F("URL, user and pass will not be stored"));

    addRowLabel(F("Try download files"));
    addSubmitButton(F("Download"), F("download"), F("red"));
  }

  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

// ********************************************************************************
// download filetype selectors
// ********************************************************************************
bool getDownloadFiletypeChecked(FileType::Enum filetype, unsigned int filenr) {
  bool isChecked = false;

  switch (filetype) {
    case FileType::CONFIG_DAT: isChecked       = ResetFactoryDefaultPreference.fetchConfigDat(); break;
    case FileType::SECURITY_DAT: isChecked     = ResetFactoryDefaultPreference.fetchSecurityDat(); break;
    case FileType::NOTIFICATION_DAT: isChecked = ResetFactoryDefaultPreference.fetchNotificationDat(); break;
    case FileType::RULES_TXT: isChecked        = ResetFactoryDefaultPreference.fetchRulesTXT(filenr); break;
  }
  return isChecked;
}

void addDownloadFiletypeCheckbox(FileType::Enum filetype, unsigned int filenr) {
  String filetype_str = getFileName(filetype, filenr);
  String label        = F("Fetch ");

  label += filetype_str;
  addRowLabel(label);
  addCheckBox(filetype_str, getDownloadFiletypeChecked(filetype, filenr));
}

void storeDownloadFiletypeCheckbox(FileType::Enum filetype, unsigned int filenr) {
  String filetype_str = getFileName(filetype, filenr);
  bool   isChecked    = isFormItemChecked(filetype_str);

  switch (filetype) {
    case FileType::CONFIG_DAT: ResetFactoryDefaultPreference.fetchConfigDat(isChecked); break;
    case FileType::SECURITY_DAT: ResetFactoryDefaultPreference.fetchSecurityDat(isChecked); break;
    case FileType::NOTIFICATION_DAT: ResetFactoryDefaultPreference.fetchNotificationDat(isChecked); break;
    case FileType::RULES_TXT: { ResetFactoryDefaultPreference.fetchRulesTXT(filenr, isChecked); break; }
  }
}

bool tryDownloadFileType(const String& url, const String& user, const String& pass, FileType::Enum filetype, unsigned int filenr) {
  if (!getDownloadFiletypeChecked(filetype, filenr)) {
    // Not selected, so not downloaded
    return false;
  }
  String filename = getFileName(filetype, filenr);
  bool   res      = false;
  String error;

  addRowLabel(filename);

  if (ResetFactoryDefaultPreference.deleteFirst()) {
    if (!fileExists(filename) || tryDeleteFile(filename)) {
      res = downloadFile(url + filename, filename, user, pass, error);
    } else {
      error = F("Could not delete existing file");
    }
  } else {
    String tmpfile = filename;
    tmpfile += F(".tmp");
    res      = downloadFile(url + filename, tmpfile, user, pass, error);

    if (res) {
      String filename_bak = filename;
      filename_bak += F("_bak");

      if (fileExists(filename) && !tryRenameFile(filename, filename_bak)) {
        res   = false;
        error = F("Could not rename to _bak");
      } else {
        // File does not exist (anymore)
        if (!tryRenameFile(tmpfile, filename)) {
          res   = false;
          error = F("Could not rename tmp file");

          if (tryRenameFile(filename_bak, filename)) {
            error += F("... reverted");
          } else {
            error += F(" Not reverted!");
          }
        }
      }
    }
  }

  if (!res) {
    addHtml(error);
  }  else {
    addHtml(F("Success"));
  }
  return res;
}

#endif // ifdef USE_SETTINGS_ARCHIVE
