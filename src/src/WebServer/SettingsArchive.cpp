#include "../WebServer/SettingsArchive.h"

#if FEATURE_SETTINGS_ARCHIVE

#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"

#include "../Globals/ResetFactoryDefaultPref.h"

#include "../Helpers/ESPEasy_FactoryDefault.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringParser.h"


// ********************************************************************************
// Web Interface to manage archived settings
// ********************************************************************************
void handle_settingsarchive() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_settingsarchive"));
  #endif

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
    for (int i = 0; i < FileType::MAX_FILETYPE; ++i) {
      const FileType::Enum ft = static_cast<FileType::Enum>(i);
      if (ft == FileType::RULES_TXT) {
        for (int i = 0; i < RULESETS_MAX; ++i) {
          storeDownloadFiletypeCheckbox(FileType::RULES_TXT, i);
        }
      } else {
        storeDownloadFiletypeCheckbox(ft);
      }
    }

    ResetFactoryDefaultPreference.deleteFirst(isFormItemChecked(F("del")));
#if FEATURE_CUSTOM_PROVISIONING
    ResetFactoryDefaultPreference.saveURL(isFormItemChecked(F("saveurl")));
    ResetFactoryDefaultPreference.allowFetchByCommand(isFormItemChecked(F("allowcommand")));
    ResetFactoryDefaultPreference.storeCredentials(isFormItemChecked(F("savecred")));
#endif
    applyFactoryDefaultPref();

    String error;
    #if FEATURE_CUSTOM_PROVISIONING
    {
      MakeProvisioningSettings(ProvisioningSettings);
      if (AllocatedProvisioningSettings()) {
        ProvisioningSettings.ResetFactoryDefaultPreference = ResetFactoryDefaultPreference.getPreference();
        if (ResetFactoryDefaultPreference.saveURL()) {
          ProvisioningSettings.setUrl(web_server.arg(F("url")));
        }
        if (ResetFactoryDefaultPreference.storeCredentials()) {
          ProvisioningSettings.setUser(web_server.arg(F("user")));
          ProvisioningSettings.setPass(web_server.arg(F("pass")));
        }
      }
      error = saveProvisioningSettings(ProvisioningSettings);
    }
    #endif

    error += SaveSettings();
    addHtmlError(error);
  }

  bool showOptions = true;

  if (web_server.hasArg(F("download"))) {
    // Try downloading files.
    // Don't use the ProvisioningSettings, as not all may be stored.
    const String url  = webArg(F("url"));
    const String user = webArg(F("user"));
    const String pass = webArg(F("pass"));

    addTableSeparator(F("Download result"), 2, 3);
    bool somethingDownloaded = false;

    for (int i = 0; i < FileType::MAX_FILETYPE; ++i) {
      const FileType::Enum ft = static_cast<FileType::Enum>(i);
      if (ft != FileType::RULES_TXT) {
        if (getDownloadFiletypeChecked(ft, 0)) {
          if (tryDownloadFileType(url, user, pass, ft)) {
            somethingDownloaded = true; 
          }
        }
      }
    }

    for (int i = 0; i < RULESETS_MAX; ++i) {
      if (getDownloadFiletypeChecked(FileType::RULES_TXT, i)) {
        if (tryDownloadFileType(url, user, pass, FileType::RULES_TXT, i)) { 
          somethingDownloaded = true; 
        }
      }
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
    addTableSeparator(F("Archive Location"), 2, 3);

    {
      String url, user, pass;

      {
        #if FEATURE_CUSTOM_PROVISIONING
        {
          MakeProvisioningSettings(ProvisioningSettings);
          if (AllocatedProvisioningSettings()) {
            loadProvisioningSettings(ProvisioningSettings);
            url = ProvisioningSettings.url;
            user = ProvisioningSettings.user;
            pass = ProvisioningSettings.pass;
          }
        }
        #endif

        if (web_server.arg(F("url")).length() != 0) {
          url = web_server.arg(F("url"));
        }
        if (web_server.arg(F("user")).length() != 0) {
          user = web_server.arg(F("user"));
        }
        if (web_server.arg(F("pass")).length() != 0) {
          pass = web_server.arg(F("pass"));
        }
      }

      addFormTextBox(F("URL with settings"), F("url"), url, 256);
      addFormNote(F("Only HTTP supported. Do not include filename. URL is allowed to contain system variables."));
      #if FEATURE_CUSTOM_PROVISIONING
      addFormCheckBox(F("Store URL"), F("saveurl"), ResetFactoryDefaultPreference.saveURL());
      #endif
      addFormTextBox(F("User"), F("user"), user, 64);
      addFormPasswordBox(F("Pass"), F("pass"), pass, 64);
      #if FEATURE_CUSTOM_PROVISIONING
      addFormCheckBox(F("Store Credentials"), F("savecred"), ResetFactoryDefaultPreference.storeCredentials());
      #endif
    }

    addTableSeparator(F("Download Settings"), 2, 3);

    addRowLabel(F("Delete First"));
    addCheckBox(F("del"), ResetFactoryDefaultPreference.deleteFirst());
    addFormNote(F("Needed on filesystem with not enough free space. Use with care!"));
    #if FEATURE_CUSTOM_PROVISIONING
    addFormCheckBox(F("Allow Fetch by Command"), F("allowcommand"), ResetFactoryDefaultPreference.allowFetchByCommand());
    addFormNote(F("Fetch files via a command does need stored URL (+ credentials)"));
    #endif

    addTableSeparator(F("Files to Download"), 2, 3);
    for (int i = 0; i < FileType::MAX_FILETYPE; ++i) {
      const FileType::Enum ft = static_cast<FileType::Enum>(i);
      if (ft != FileType::RULES_TXT) {
        addDownloadFiletypeCheckbox(ft);
      }
    }

    for (int i = 0; i < RULESETS_MAX; ++i) {
      addDownloadFiletypeCheckbox(FileType::RULES_TXT, i);
    }

    html_TR_TD();
    html_TD();
    addSubmitButton(F("Save Preferences"), F("savepref"));

    addFormSeparator(2);

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
void addDownloadFiletypeCheckbox(FileType::Enum filetype, unsigned int filenr) {
  const String filetype_str = getFileName(filetype, filenr);
  String label        = F("Fetch ");

  label += filetype_str;
  addRowLabel(label);
  addCheckBox(filetype_str, getDownloadFiletypeChecked(filetype, filenr));
}

void storeDownloadFiletypeCheckbox(FileType::Enum filetype, unsigned int filenr) {
  const bool isChecked = isFormItemChecked(getFileName(filetype, filenr));

  switch (filetype) {
    case FileType::CONFIG_DAT: ResetFactoryDefaultPreference.fetchConfigDat(isChecked); break;
    case FileType::SECURITY_DAT: ResetFactoryDefaultPreference.fetchSecurityDat(isChecked); break;
    case FileType::NOTIFICATION_DAT: ResetFactoryDefaultPreference.fetchNotificationDat(isChecked); break;
    case FileType::RULES_TXT: { ResetFactoryDefaultPreference.fetchRulesTXT(filenr, isChecked); break; }
    case FileType::PROVISIONING_DAT: { ResetFactoryDefaultPreference.fetchProvisioningDat(isChecked); break; }
    case FileType::FIRMWARE:  // FIXME TD-er: Still have to decide what to do with protecting firmware downloads
    case FileType::MAX_FILETYPE: 
      break;
    
  }
}

bool tryDownloadFileType(const String& url, const String& user, const String& pass, FileType::Enum filetype, unsigned int filenr) {
  const String filename = getFileName(filetype, filenr);
  addRowLabel(filename);
  const String error = downloadFileType(url, user, pass, filetype, filenr);
  if (error.length() == 0) {
    addHtml(F("Success"));
    return true;
  }
  addHtml(error);
  return false;
}

#endif // if FEATURE_SETTINGS_ARCHIVE
