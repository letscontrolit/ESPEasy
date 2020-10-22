#include "../WebServer/UploadPage.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/HTML_wrappers.h"

#include "../Helpers/ESPEasy_Storage.h"

#include "../../ESPEasy-Globals.h"


#ifdef WEBSERVER_UPLOAD

// ********************************************************************************
// Web Interface upload page
// ********************************************************************************
byte uploadResult = 0;
void handle_upload() {
  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();

  addHtml(F(
            "<form enctype='multipart/form-data' method='post'><p>Upload settings file:<br><input type='file' name='datafile' size='40'></p><div><input class='button link' type='submit' value='Upload'></div><input type='hidden' name='edit' value='1'></form>"));
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
  printWebString = "";
  printToWeb     = false;
}

// ********************************************************************************
// Web Interface upload page
// ********************************************************************************
void handle_upload_post() {
  checkRAM(F("handle_upload_post"));

  if (!isLoggedIn()) { return; }

  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();


  if (uploadResult == 1)
  {
    addHtml(F("Upload OK!<BR>You may need to reboot to apply all settings..."));
    LoadSettings();
  }

  if (uploadResult == 2) {
    addHtml(F("<font color=\"red\">Upload file invalid!</font>"));
  }

  if (uploadResult == 3) {
    addHtml(F("<font color=\"red\">No filename!</font>"));
  }


  addHtml(F("Upload finished"));
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
  printWebString = "";
  printToWeb     = false;
}

#ifdef WEBSERVER_NEW_UI
void handle_upload_json() {
  checkRAM(F("handle_upload_post"));
  uint8_t result = uploadResult;

  if (!isLoggedIn()) { result = 255; }

  TXBuffer.startJsonStream();
  addHtml("{");
  stream_next_json_object_value(F("status"), String(result));
  addHtml("}");

  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface upload handler
// ********************************************************************************
fs::File uploadFile;
void handleFileUpload() {
  checkRAM(F("handleFileUpload"));

  if (!isLoggedIn()) { return; }

  static boolean valid = false;

  HTTPUpload& upload = web_server.upload();

  if (upload.filename.c_str()[0] == 0)
  {
    uploadResult = 3;
    return;
  }

  if (upload.status == UPLOAD_FILE_START)
  {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("Upload: START, filename: ");
      log += upload.filename;
      addLog(LOG_LEVEL_INFO, log);
    }
    valid        = false;
    uploadResult = 0;
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    // first data block, if this is the config file, check PID/Version
    if (upload.totalSize == 0)
    {
      if (strcasecmp(upload.filename.c_str(), FILE_CONFIG) == 0)
      {
        struct TempStruct {
          unsigned long PID;
          int           Version;
        } Temp;

        for (unsigned int x = 0; x < sizeof(struct TempStruct); x++)
        {
          byte b = upload.buf[x];
          memcpy((byte *)&Temp + x, &b, 1);
        }

        if ((Temp.Version == VERSION) && (Temp.PID == ESP_PROJECT_PID)) {
          valid = true;
        }
      }
      else
      {
        // other files are always valid...
        valid = true;
      }

      if (valid)
      {
        String filename;
#if defined(ESP32)
        filename += '/';
#endif // if defined(ESP32)
        filename += upload.filename;

        // once we're safe, remove file and create empty one...
        tryDeleteFile(filename);
        uploadFile = tryOpenFile(filename.c_str(), "w");

        // dont count manual uploads: flashCount();
      }
    }

    if (uploadFile) { uploadFile.write(upload.buf, upload.currentSize); }

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("Upload: WRITE, Bytes: ");
      log += upload.currentSize;
      addLog(LOG_LEVEL_INFO, log);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (uploadFile) { uploadFile.close(); }

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("Upload: END, Size: ");
      log += upload.totalSize;
      addLog(LOG_LEVEL_INFO, log);
    }
  }

  if (valid) {
    uploadResult = 1;
  }
  else {
    uploadResult = 2;
  }
}

#endif // ifdef WEBSERVER_UPLOAD
