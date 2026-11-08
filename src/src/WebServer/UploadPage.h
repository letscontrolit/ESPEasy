#pragma once

#include "../WebServer/common.h"

#ifdef WEBSERVER_UPLOAD
# include "../Helpers/ESPEasy_Storage.h"

// ********************************************************************************
// Web Interface upload page
// ********************************************************************************
enum class uploadResult_e {
  // Int values are used in JSON, so keep them numbered like this.
  UploadStarted = 0,
  Success       = 1,
  InvalidFile   = 2,
  NoFilename    = 3,
  SuccessReboot = 4, // Success, reboot strongly advised (red emphasis in message)
};

extern uploadResult_e uploadResult;

void handle_upload();

// ********************************************************************************
// Web Interface upload page
// ********************************************************************************
void handle_upload_post();

# ifdef WEBSERVER_NEW_UI
void handle_upload_json();

# endif // ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface upload handler
// ********************************************************************************
extern fs::File uploadFile;
void handleFileUpload();
# if FEATURE_SD
void handleSDFileUpload();
# endif // if FEATURE_SD
#endif // ifdef WEBSERVER_UPLOAD