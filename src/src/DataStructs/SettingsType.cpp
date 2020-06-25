#include "SettingsType.h"

#include "ControllerSettingsStruct.h"
#include "NotificationSettingsStruct.h"
#include "SecurityStruct.h"
#include "StorageLayout.h"
#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/Settings.h"
#include "../../ESPEasy-Globals.h"

String SettingsType::getSettingsTypeString(Enum settingsType) {
  switch (settingsType) {
    case BasicSettings_Type:            return F("Settings");
    case TaskSettings_Type:             return F("TaskSettings");
    case CustomTaskSettings_Type:       return F("CustomTaskSettings");
    case ControllerSettings_Type:       return F("ControllerSettings");
    case CustomControllerSettings_Type: return F("CustomControllerSettings");
    case NotificationSettings_Type:     return F("NotificationSettings");
    case SecuritySettings_Type:         return F("SecuritySettings");
    case ExtdControllerCredentials_Type: return F("ExtendedControllerCredentials");
    default:
      break;
  }
  return "";
}

/********************************************************************************************\
   Offsets in settings files
 \*********************************************************************************************/
bool SettingsType::getSettingsParameters(Enum settingsType, int index, int& max_index, int& offset, int& max_size, int& struct_size) {
  // The defined offsets should be used with () just in case they are the result of a formula in the defines.
  struct_size = 0;

  switch (settingsType) {
    case BasicSettings_Type:
    {
      max_index   = 1;
      offset      = 0;
      max_size    = (DAT_BASIC_SETTINGS_SIZE);
      struct_size = sizeof(SettingsStruct);
      break;
    }
    case TaskSettings_Type:
    {
      max_index   = TASKS_MAX;
      offset      = (DAT_OFFSET_TASKS) + (index * (DAT_TASKS_DISTANCE));
      max_size    = DAT_TASKS_SIZE;
      struct_size = sizeof(ExtraTaskSettingsStruct);
      break;
    }
    case CustomTaskSettings_Type:
    {
      getSettingsParameters(TaskSettings_Type, index, max_index, offset, max_size, struct_size);
      offset  += (DAT_TASKS_CUSTOM_OFFSET);
      max_size = DAT_TASKS_CUSTOM_SIZE;

      // struct_size may differ.
      struct_size = 0;
      break;
    }
    case ControllerSettings_Type:
    {
      max_index   = CONTROLLER_MAX;
      offset      = (DAT_OFFSET_CONTROLLER) + (index * (DAT_CONTROLLER_SIZE));
      max_size    = DAT_CONTROLLER_SIZE;
      struct_size = sizeof(ControllerSettingsStruct);
      break;
    }
    case CustomControllerSettings_Type:
    {
      max_index = CONTROLLER_MAX;
      offset    = (DAT_OFFSET_CUSTOM_CONTROLLER) + (index * (DAT_CUSTOM_CONTROLLER_SIZE));
      max_size  = DAT_CUSTOM_CONTROLLER_SIZE;

      // struct_size may differ.
      struct_size = 0;
      break;
    }
    case NotificationSettings_Type:
    {
      max_index   = NOTIFICATION_MAX;
      offset      = index * (DAT_NOTIFICATION_SIZE);
      max_size    = DAT_NOTIFICATION_SIZE;
      struct_size = sizeof(NotificationSettingsStruct);
      break;
    }
    case SecuritySettings_Type:
    {
      max_index   = 1;
      offset      = 0;
      max_size    = DAT_SECURITYSETTINGS_SIZE;
      struct_size = sizeof(SecurityStruct);
      break;
    }
    case ExtdControllerCredentials_Type:
    {
      max_index = 1;
      offset    = DAT_EXTDCONTR_CRED_OFFSET;
      max_size  = DAT_EXTDCONTR_CRED_SIZE;

      // struct_size may differ.
      struct_size = 0;
      break;
    }
    case SettingsType_MAX:
    {
      max_index = -1;
      offset    = -1;
      return false;
    }
  }
  return index >= 0 && index < max_index;
}

bool SettingsType::getSettingsParameters(Enum settingsType, int index, int& offset, int& max_size) {
  int max_index = -1;
  int struct_size;

  if (!getSettingsParameters(settingsType, index, max_index, offset, max_size, struct_size)) {
    return false;
  }

  if ((index >= 0) && (index < max_index)) { return true; }
  offset = -1;
  return false;
}

int SettingsType::getMaxFilePos(Enum settingsType) {
  int max_index, offset, max_size;
  int struct_size = 0;

  getSettingsParameters(settingsType, 0,             max_index, offset, max_size, struct_size);
  getSettingsParameters(settingsType, max_index - 1, offset,    max_size);
  return offset + max_size - 1;
}

int SettingsType::getFileSize(Enum settingsType) {
  SettingsType::SettingsFileEnum file_type = SettingsType::getSettingsFile(settingsType);
  int max_file_pos                         = 0;

  for (int st = 0; st < SettingsType_MAX; ++st) {
    if (SettingsType::getSettingsFile(static_cast<Enum>(st)) == file_type) {
      int filePos = SettingsType::getMaxFilePos(static_cast<Enum>(st));

      if (filePos > max_file_pos) {
        max_file_pos = filePos;
      }
    }
  }
  return max_file_pos;
}

#ifndef BUILD_MINIMAL_OTA
unsigned int SettingsType::getSVGcolor(Enum settingsType) {
  switch (settingsType) {
    case BasicSettings_Type:
      return 0x5F0A87;
    case TaskSettings_Type:
      return 0xEE6352;
    case CustomTaskSettings_Type:
      return 0x59CD90;
    case ControllerSettings_Type:
      return 0x3FA7D6;
    case CustomControllerSettings_Type:
      return 0xFAC05E;
    case NotificationSettings_Type:
      return 0xF79D84;

    case SecuritySettings_Type:
      return 0xff00a2;
    case ExtdControllerCredentials_Type:
      return 0xc300ff;
    case SettingsType_MAX:
      break;
  }
  return 0;
}
#endif

SettingsType::SettingsFileEnum SettingsType::getSettingsFile(Enum settingsType)
{
  switch (settingsType) {
    case BasicSettings_Type:
    case TaskSettings_Type:
    case CustomTaskSettings_Type:
    case ControllerSettings_Type:
    case CustomControllerSettings_Type:
      return FILE_CONFIG_type;
    case NotificationSettings_Type:
      return FILE_NOTIFICATION_type;
    case SecuritySettings_Type:
    case ExtdControllerCredentials_Type:
      return FILE_SECURITY_type;

    case SettingsType_MAX:
      break;
  }
  return FILE_UNKNOWN_type;
}

String SettingsType::getSettingsFileName(Enum settingsType) {
  SettingsType::SettingsFileEnum file_type = getSettingsFile(settingsType);

  switch (file_type) {
    case FILE_CONFIG_type:        return F(FILE_CONFIG);
    case FILE_NOTIFICATION_type:  return F(FILE_NOTIFICATION);
    case FILE_SECURITY_type:      return F(FILE_SECURITY);
    case FILE_UNKNOWN_type:       break;
  }
  return "";
}
