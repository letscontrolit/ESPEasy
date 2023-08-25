#include "../WebServer/UploadPage.h"

#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/HTML_wrappers.h"

#include "../Globals/Cache.h"
#include "../Helpers/ESPEasy_Storage.h"
#if FEATURE_TARSTREAM_SUPPORT
# include "../Helpers/TarStream.h"
#endif // if FEATURE_TARSTREAM_SUPPORT
#include "../../ESPEasy-Globals.h"


#ifdef WEBSERVER_UPLOAD

# ifndef FEATURE_UPLOAD_CLEANUP_CONFIG
#  define FEATURE_UPLOAD_CLEANUP_CONFIG 1 // Enable/Disable removing of extcfg<tasknr>.dat files when a .tar file is uploaded having
                                          // a valid config.dat included
# endif // ifndef FEATURE_UPLOAD_CLEANUP_CONFIG

// ********************************************************************************
// Web Interface upload page
// ********************************************************************************
uploadResult_e uploadResult = uploadResult_e::UploadStarted;

void handle_upload() {
  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  addHtml(F(
            "<form enctype='multipart/form-data' method='post'><p>Upload settings file:<br><input type='file' name='datafile' size='40'></p><div><input class='button link' type='submit' value='Upload'></div><input type='hidden' name='edit' value='1'></form>"));
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
  printWebString = String();
  printToWeb     = false;
}

// ********************************************************************************
// Web Interface upload page
// ********************************************************************************
void handle_upload_post() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_upload_post"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn()) { return; }

  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  switch (uploadResult) {
    case uploadResult_e::Success:
    case uploadResult_e::SuccessReboot:
      addHtml(F("Upload OK!<BR>"));

      if (uploadResult_e::SuccessReboot == uploadResult) { // Enable string de-duplication
        addHtml(F("<font color='red'>"));
        addHtml(F("You REALLY need to reboot to apply all settings..."));
        addHtml(F("</font><BR>"));
      } else {
        addHtml(F("You may"));
        addHtml(F(" need to reboot to apply all settings..."));
      }
      LoadSettings();
      break;
    case uploadResult_e::InvalidFile:
      addHtml(F("<font color='red'>"));
      addHtml(F("Upload file invalid!"));
      addHtml(F("</font><BR>"));
      break;
    case uploadResult_e::NoFilename:
      addHtml(F("<font color='red'>"));
      addHtml(F("No filename!"));
      addHtml(F("</font><BR>"));
      break;
    case uploadResult_e::UploadStarted:
      break;
  }

  addHtml(F("Upload finished"));
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
  printWebString = String();
  printToWeb     = false;
}

# ifdef WEBSERVER_NEW_UI
void handle_upload_json() {
  #  ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_upload_post"));
  #  endif // ifndef BUILD_NO_RAM_TRACKER
  uint8_t result = static_cast<int>(uploadResult);

  if (!isLoggedIn()) { result = 255; }

  TXBuffer.startJsonStream();
  addHtml('{');
  stream_next_json_object_value(F("status"), result);
  addHtml('}');

  TXBuffer.endStream();
}

# endif // WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface upload handler
// ********************************************************************************
fs::File uploadFile;
# if FEATURE_TARSTREAM_SUPPORT
TarStream *tarStream = nullptr;
# endif // if FEATURE_TARSTREAM_SUPPORT
void handleFileUpload() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handleFileUpload"));
  # endif // ifndef BUILD_NO_RAM_TRACKER
  handleFileUploadBase(false);
}

# if FEATURE_SD
void handleSDFileUpload() {
  #  ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handleSDFileUpload"));
  #  endif // ifndef BUILD_NO_RAM_TRACKER
  handleFileUploadBase(true);
}

# endif // if FEATURE_SD

void handleFileUploadBase(bool toSDcard) {
  if (!isLoggedIn()) { return; }

  static boolean valid   = false;
  bool receivedConfigDat = false;

  HTTPUpload& upload = web_server.upload();

  if (upload.filename.c_str()[0] == 0)
  {
    uploadResult = uploadResult_e::NoFilename;
    return;
  }

  if (upload.status == UPLOAD_FILE_START)
  {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, concat(F("Upload: START, filename: "), upload.filename));
    }
    valid        = false;
    uploadResult = uploadResult_e::UploadStarted;
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    // first data block, if this is the config file, check PID/Version
    if (upload.totalSize == 0)
    {
      if (matchFileType(upload.filename, FileType::CONFIG_DAT)) {
        valid             = validateUploadConfigDat(upload.buf);
        receivedConfigDat = true;
      } else {
        // other files are always assumed valid...
        valid = true;
      }

      if (valid) {
        String filename = patch_fname(upload.filename);

        # if FEATURE_TARSTREAM_SUPPORT

        if ((filename.length() > 3) && filename.substring(filename.length() - 4).equalsIgnoreCase(F(".tar"))) {
          tarStream = new TarStream(filename, toSDcard ? FileDestination_e::SD : FileDestination_e::ANY);
          #  ifndef BUILD_NO_DEBUG
          addLogMove(LOG_LEVEL_INFO, concat(F("Upload: TAR Processing .tar file: "), filename));
          #  endif // ifndef BUILD_NO_DEBUG
        } else
        # endif // if FEATURE_TARSTREAM_SUPPORT
        {
          // once we're safe, remove file and create empty one...
          tryDeleteFile(filename, toSDcard ? FileDestination_e::SD : FileDestination_e::ANY);
          uploadFile = tryOpenFile(filename.c_str(), "w", toSDcard ? FileDestination_e::SD : FileDestination_e::ANY);
        }

        // dont count manual uploads: flashCount();
      }
    }

    # if FEATURE_TARSTREAM_SUPPORT

    if (nullptr != tarStream) {
      tarStream->write(upload.buf, upload.currentSize);
    } else
    # endif // if FEATURE_TARSTREAM_SUPPORT
    {
      if (uploadFile) { uploadFile.write(upload.buf, upload.currentSize); }
    }

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, concat(F("Upload: WRITE, Bytes: "), upload.currentSize));
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    # if FEATURE_TARSTREAM_SUPPORT

    if (nullptr != tarStream) {
      tarStream->flush();
      #  if FEATURE_EXTENDED_CUSTOM_SETTINGS && FEATURE_UPLOAD_CLEANUP_CONFIG

      // If we have received a valid config.dat, and extended custom settings is available,
      // delete all NOT included extcfg<tasknr>.dat files
      // FIXME Keep this feature enabled ??? (the extra files _can't_ be deleted manually)
      if (tarStream->isFileIncluded(getFileName(FileType::CONFIG_DAT))) { // Is config.dat included?
        receivedConfigDat = true;

        for (uint8_t n = 1; n <= TASKS_MAX; n++) {
          const String tmpfile = strformat(F(DAT_TASKS_CUSTOM_EXTENSION_FILEMASK), n);

          if (!tarStream->isFileIncluded(tmpfile) &&
              tryDeleteFile(tmpfile)) {
            addLogMove(LOG_LEVEL_INFO, concat(F("Upload: Removing not included extended settings: "), tmpfile));
          }
        }
      }
      #  endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS && FEATURE_UPLOAD_CLEANUP_CONFIG
      delete tarStream;
    } else
    # endif // if FEATURE_TARSTREAM_SUPPORT
    {
      if (uploadFile) { uploadFile.close(); }
    }

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, concat(F("Upload: END, Size: "), upload.totalSize));
    }
  }

  if (valid) {
    if (receivedConfigDat) {
      uploadResult = uploadResult_e::SuccessReboot;
    } else {
      uploadResult = uploadResult_e::Success;
    }
  }
  else {
    uploadResult = uploadResult_e::InvalidFile;
  }
}

#endif // ifdef WEBSERVER_UPLOAD
