#include "../DataStructs/ProvisioningStruct.h"

#if FEATURE_CUSTOM_PROVISIONING

# include "../Helpers/StringConverter.h"
# include "../Helpers/Hardware.h"

ProvisioningStruct::ProvisioningStruct() {
  ZERO_FILL(user);
  ZERO_FILL(pass);
  ZERO_FILL(url);
}

void ProvisioningStruct::validate() {
  ZERO_TERMINATE(user);
  ZERO_TERMINATE(pass);
  ZERO_TERMINATE(url);
}

bool ProvisioningStruct::matchingFlashSize() const
{
  return modelMatchingFlashSize(ResetFactoryDefaultPreference.getDeviceModel());
}

bool ProvisioningStruct::setUser(const String& username)
{
  return safe_strncpy(user, username, sizeof(user));
}

bool ProvisioningStruct::setPass(const String& password)
{
  return safe_strncpy(pass, password, sizeof(pass));
}

bool ProvisioningStruct::setUrl(const String& url_str)
{
  return safe_strncpy(url, url_str, sizeof(url));
}

bool ProvisioningStruct::fetchFileTypeAllowed(FileType::Enum filetype, unsigned int filenr) const
{
  switch (filetype) {
    case FileType::CONFIG_DAT:       return allowedFlags.allowFetchConfigDat;
    case FileType::SECURITY_DAT:     return allowedFlags.allowFetchSecurityDat;
    case FileType::NOTIFICATION_DAT: return allowedFlags.allowFetchNotificationDat;
    case FileType::RULES_TXT:        return (filenr < RULESETS_MAX) && bitRead(allowedFlags.allowFetchRules, filenr);
    case FileType::PROVISIONING_DAT: return allowedFlags.allowFetchProvisioningDat;

    case FileType::MAX_FILETYPE:
      break;
  }
  return false;
}

void ProvisioningStruct::setFetchFileTypeAllowed(FileType::Enum filetype, unsigned int filenr, bool checked)
{
  switch (filetype) {
    case FileType::CONFIG_DAT:       allowedFlags.allowFetchConfigDat       = checked; break;
    case FileType::SECURITY_DAT:     allowedFlags.allowFetchSecurityDat     = checked; break;
    case FileType::NOTIFICATION_DAT: allowedFlags.allowFetchNotificationDat = checked; break;
    case FileType::PROVISIONING_DAT: allowedFlags.allowFetchProvisioningDat = checked; break;
    case FileType::RULES_TXT:

      if (filenr < RULESETS_MAX) {
        bitWrite(allowedFlags.allowFetchRules, filenr, checked);
      }
      break;

    case FileType::MAX_FILETYPE:
      break;
  }
}

#endif // if FEATURE_CUSTOM_PROVISIONING
