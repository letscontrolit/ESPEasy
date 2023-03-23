#ifndef WEBSERVER_WEBSERVER_SETTINGSARCHIVE_H
#define WEBSERVER_WEBSERVER_SETTINGSARCHIVE_H

#include "../WebServer/common.h"

#if FEATURE_SETTINGS_ARCHIVE

#include "../DataTypes/ESPEasyFileType.h"

#if FEATURE_CUSTOM_PROVISIONING
#include "../DataStructs/ProvisioningStruct.h"
#endif



// ********************************************************************************
// Web Interface to manage archived settings
// ********************************************************************************
void handle_settingsarchive();

// ********************************************************************************
// download filetype selectors
// ********************************************************************************
void addDownloadFiletypeCheckbox(FileType::Enum filetype, unsigned int filenr = 0);

void storeDownloadFiletypeCheckbox(FileType::Enum filetype, unsigned int filenr = 0);

# if FEATURE_CUSTOM_PROVISIONING
void addAllowFiletypeCheckbox(const ProvisioningStruct& ProvisioningSettings, FileType::Enum filetype, unsigned int filenr = 0);

void storeAllowFiletypeCheckbox(ProvisioningStruct& ProvisioningSettings, FileType::Enum filetype, unsigned int filenr = 0);
# endif

bool tryDownloadFileType(const String& url, const String& user, const String& pass, FileType::Enum filetype, unsigned int filenr = 0);

#endif // if FEATURE_SETTINGS_ARCHIVE

#endif