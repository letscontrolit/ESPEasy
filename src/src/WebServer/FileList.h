#ifndef WEBSERVER_WEBSERVER_FILELIST_H
#define WEBSERVER_WEBSERVER_FILELIST_H

#include "../WebServer/common.h"


#ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface file list
// ********************************************************************************
void handle_filelist_json();

#endif // WEBSERVER_NEW_UI

#ifdef WEBSERVER_FILELIST
void handle_filelist();

void handle_filelist_add_file(const String& filename, int filesize, int startIdx);

void handle_filelist_buttons(int start_prev, int start_next, bool cacheFilesPresent);

#endif // ifdef WEBSERVER_FILELIST

// ********************************************************************************
// Web Interface SD card file and directory list
// ********************************************************************************
#ifdef FEATURE_SD
void handle_SDfilelist();

#endif // ifdef FEATURE_SD



#endif