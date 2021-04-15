#ifndef WEBSERVER_WEBSERVER_UPLOADPAGE_H
#define WEBSERVER_WEBSERVER_UPLOADPAGE_H

#include "../WebServer/common.h"

#ifdef WEBSERVER_UPLOAD

// ********************************************************************************
// Web Interface upload page
// ********************************************************************************
enum class uploadResult_e {
    // Int values are used in JSON, so keep them numbered like this.
    UploadStarted = 0,
    Success = 1,
    InvalidFile = 2,
    NoFilename = 3
};

extern uploadResult_e uploadResult;

void handle_upload();

// ********************************************************************************
// Web Interface upload page
// ********************************************************************************
void handle_upload_post();

#ifdef WEBSERVER_NEW_UI
void handle_upload_json();

#endif // WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface upload handler
// ********************************************************************************
extern fs::File uploadFile;
void handleFileUpload();

#endif 

#endif