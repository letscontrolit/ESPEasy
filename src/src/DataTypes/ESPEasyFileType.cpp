#include "../DataTypes/ESPEasyFileType.h"

#include "../../ESPEasy_common.h"

#include "../CustomBuild/StorageLayout.h"

#include "../Globals/ResetFactoryDefaultPref.h"

#include "../Helpers/StringConverter.h"

bool matchFileType(const String& filename, FileType::Enum filetype)
{
  if (filename.startsWith(F("/"))) {
    return matchFileType(filename.substring(1), filetype);
  }
  return filename.equalsIgnoreCase(getFileName(filetype));
}

bool isProtectedFileType(const String& filename)
{
  #if FEATURE_EXTENDED_CUSTOM_SETTINGS
  bool isTaskSpecificConfig = false;
  const String fname        = filename.substring(filename.startsWith(F("/")) ? 1 : 0);
  const String mask         = F(DAT_TASKS_CUSTOM_EXTENSION_FILEMASK);
  const int8_t mPerc        = mask.indexOf('%');

  if ((mPerc > -1) && fname.startsWith(mask.substring(0, mPerc))) {
    for (uint8_t n = 0; n < TASKS_MAX && !isTaskSpecificConfig; ++n) {
      isTaskSpecificConfig |= (fname.equalsIgnoreCase(strformat(mask, n + 1)));
    }
  }
  #endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS

  return
    #if FEATURE_EXTENDED_CUSTOM_SETTINGS
    isTaskSpecificConfig || // Support for extcfgNN.dat
    #endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS
    matchFileType(filename, FileType::CONFIG_DAT) ||
    matchFileType(filename, FileType::SECURITY_DAT) ||
    matchFileType(filename, FileType::NOTIFICATION_DAT) ||
    matchFileType(filename, FileType::PROVISIONING_DAT);
}

const __FlashStringHelper* getFileName(FileType::Enum filetype) {
  switch (filetype)
  {
    case FileType::CONFIG_DAT:       return F("config.dat");
    case FileType::NOTIFICATION_DAT: return F("notification.dat");
    case FileType::SECURITY_DAT:     return F("security.dat");
    case FileType::PROVISIONING_DAT: return F("provisioning.dat");
    case FileType::RULES_TXT:
      // Use getRulesFileName
      break;

    case FileType::MAX_FILETYPE:
      break;
  }
  return F("");
}

String getFileName(FileType::Enum filetype, unsigned int filenr) {
  if (filetype == FileType::RULES_TXT) {
    return getRulesFileName(filenr);
  }
  return getFileName(filetype);
}

// filenr = 0...3 for files rules1.txt ... rules4.txt
String getRulesFileName(unsigned int filenr) {
  String result;

  if (filenr < RULESETS_MAX) {
    result += F("rules");
    result += filenr + 1;
    result += F(".txt");
  }
  return result;
}

bool getDownloadFiletypeChecked(FileType::Enum filetype, unsigned int filenr) {
  switch (filetype) {
    case FileType::CONFIG_DAT:       return ResetFactoryDefaultPreference.fetchConfigDat();
    case FileType::SECURITY_DAT:     return ResetFactoryDefaultPreference.fetchSecurityDat();
    case FileType::NOTIFICATION_DAT: return ResetFactoryDefaultPreference.fetchNotificationDat();
    case FileType::RULES_TXT:        return ResetFactoryDefaultPreference.fetchRulesTXT(filenr);
    case FileType::PROVISIONING_DAT: return ResetFactoryDefaultPreference.fetchProvisioningDat();
      break;

    case FileType::MAX_FILETYPE:
      break;
  }
  return false;
}
