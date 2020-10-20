#ifndef WEBSERVER_WEBSERVER_UPLOADPAGE_H
#define WEBSERVER_WEBSERVER_UPLOADPAGE_H

#include "../WebServer/common.h"

#ifdef WEBSERVER_UPLOAD

// ********************************************************************************
// Web Interface upload page
// ********************************************************************************
extern byte uploadResult;
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