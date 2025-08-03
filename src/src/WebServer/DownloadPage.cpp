#include "../WebServer/DownloadPage.h"

#ifdef WEBSERVER_DOWNLOAD

# include "../WebServer/ESPEasy_WebServer.h"
# include "../DataTypes/SettingsType.h"
# include "../Globals/ESPEasy_time.h"
# include "../Globals/Settings.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/StringGenerator_System.h"

# if FEATURE_TARSTREAM_SUPPORT
#  include "../Helpers/TarStream.h"
# endif // if FEATURE_TARSTREAM_SUPPORT

// ********************************************************************************
// Web Interface download page
// ********************************************************************************
void handle_download() {
# if FEATURE_TARSTREAM_SUPPORT
  handle_config_download(false, false);
}

void handle_full_backup() {
  handle_config_download(true, false);
}

void handle_full_backup_no_usr_pwd() {
  handle_config_download(true, true);
}

void handle_config_download(bool fullBackup,
                            bool noCreds) {
# else // if FEATURE_TARSTREAM_SUPPORT
  const bool noCreds = false;
# endif // if FEATURE_TARSTREAM_SUPPORT
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_download"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;

  //  TXBuffer.startStream();
  //  sendHeadandTail_stdtemplate(_HEAD);


  fs::File dataFile = tryOpenFile(getFileName(FileType::CONFIG_DAT), "r");

  if (!dataFile) {
    return;
  }

  String str = F("attachment; filename=");
  # if FEATURE_TARSTREAM_SUPPORT

  if (fullBackup) {
    str += F("backup_");
  } else
  # endif // if FEATURE_TARSTREAM_SUPPORT
  {
    str += F("config_");
  }

  if (noCreds) {
    str += F("no_creds_");
  }
  str += strformat(F("%s_U%d_Build%s_"),
                   Settings.getName().c_str(),
                   Settings.Unit,
                   getSystemBuildString().c_str());

  if (node_time.systemTimePresent())
  {
    str += node_time.getDateTimeString('\0', '\0', '\0');
  }

  # if FEATURE_TARSTREAM_SUPPORT
  bool useTarFile       = false;
  const int  equalsSign = str.indexOf('=');
  TarStream *tarStream  = (fullBackup || !Settings.DisableSaveConfigAsTar())
    ? new TarStream(str.substring(equalsSign + 1) + F(".tar"))
    : nullptr;

  if (fullBackup && (nullptr != tarStream)) {
    const String security_dat = getFileName(FileType::SECURITY_DAT);
    #  if defined(ESP8266)

    fs::Dir dir = ESPEASY_FS.openDir("");

    while (dir.next()) {
      fs::File file = dir.openFile("r");

      if (file) {
        if (!noCreds || (noCreds && (0 != strncasecmp(file.name(), security_dat.c_str(), security_dat.length())))) {
          tarStream->addFile(file.name(), file.size());
        }
        file.close();
      }
    }
    #  endif // if defined(ESP8266)
    #  if defined(ESP32)
    fs::File root = ESPEASY_FS.open("/");
    fs::File file = root.openNextFile();

    while (file) {
      if (!file.isDirectory()) {
        if (!noCreds || (noCreds && (0 != strncasecmp(file.name(), security_dat.c_str(), security_dat.length())))) {
          tarStream->addFile(file.name(), file.size());
        }
      }
      file = root.openNextFile();
    }
    #  endif // if defined(ESP32)
  } else {
    if (nullptr != tarStream) {
      tarStream->addFile(dataFile.name(), dataFile.size());

      #  if FEATURE_EXTENDED_CUSTOM_SETTINGS

      // extcfg<tasknr>.dat files
      for (uint8_t n = 0; n < TASKS_MAX; ++n) {
        tarStream->addFileIfExists(SettingsType::getSettingsFileName(SettingsType::Enum::CustomTaskSettings_Type, n));
      }
      #  endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS

      // other config files
      tarStream->addFileIfExists(getFileName(FileType::NOTIFICATION_DAT));
      tarStream->addFileIfExists(getFileName(FileType::PROVISIONING_DAT));

      if (!noCreds) {
        tarStream->addFileIfExists(getFileName(FileType::SECURITY_DAT));
      }

      // rules<n>.txt files
      for (unsigned int rf = 0; rf < RULESETS_MAX; ++rf) {
        tarStream->addFileIfExists(getRulesFileName(rf));
      }
    }
  }

  if (nullptr != tarStream) {
    #  ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_INFO, strformat(F("Download: %d file(s) added to .tar. Size: %d bytes, filebytes: %d"),
                                     tarStream->getFileCount(), tarStream->size(), tarStream->getFilesSizes()));
    #  endif // ifndef BUILD_NO_DEBUG
    useTarFile = tarStream->getFileCount() > 1; // We should at least have config.dat, so ignore that
  }

  if (useTarFile) {
    str += F(".tar");
  } else

  # endif // if FEATURE_TARSTREAM_SUPPORT
  {
    str += F(".dat"); // This is in the 'else' part of the 'if' above!
  }

  sendHeader(F("Content-Disposition"), str);

  # if FEATURE_TARSTREAM_SUPPORT

  if (useTarFile) {
    web_server.streamFile(*tarStream, F("application/octet-stream"));
  } else {
    web_server.streamFile(dataFile, F("application/octet-stream"));
  }

  if (nullptr != tarStream) {
    delete tarStream;
  }

  # else // if FEATURE_TARSTREAM_SUPPORT
  web_server.streamFile(dataFile, F("application/octet-stream"));
  # endif // if FEATURE_TARSTREAM_SUPPORT
  dataFile.close();
}

#endif // ifdef WEBSERVER_DOWNLOAD
