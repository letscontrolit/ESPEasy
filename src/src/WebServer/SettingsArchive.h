#ifndef WEBSERVER_WEBSERVER_SETTINGSARCHIVE_H
#define WEBSERVER_WEBSERVER_SETTINGSARCHIVE_H

#include "../WebServer/common.h"

#ifdef USE_SETTINGS_ARCHIVE

#include "../Helpers/StringProvider.h"

// ********************************************************************************
// Web Interface to manage archived settings
// ********************************************************************************
void handle_settingsarchive();

// ********************************************************************************
// download filetype selectors
// ********************************************************************************
bool getDownloadFiletypeChecked(FileType::Enum filetype, unsigned int filenr);

void addDownloadFiletypeCheckbox(FileType::Enum filetype, unsigned int filenr);

void storeDownloadFiletypeCheckbox(FileType::Enum filetype, unsigned int filenr);

bool tryDownloadFileType(const String& url, const String& user, const String& pass, FileType::Enum filetype, unsigned int filenr);

#endif // ifdef USE_SETTINGS_ARCHIVE

#endif