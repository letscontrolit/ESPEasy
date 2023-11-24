#include "../DataTypes/ESPEasyFileType.h"

#include "../../ESPEasy_common.h"

#include "../Globals/ResetFactoryDefaultPref.h"

bool matchFileType(const String& filename, FileType::Enum filetype)
{
  if (filename.startsWith(F("/"))) {
    return matchFileType(filename.substring(1), filetype);
  }
  return filename.equalsIgnoreCase(getFileName(filetype));
}

bool isProtectedFileType(const String& filename)
{
  return matchFileType(filename, FileType::CONFIG_DAT) ||
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
