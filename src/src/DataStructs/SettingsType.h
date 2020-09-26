#ifndef DATASTRUCTS_SETTINGSTYPE_H
#define DATASTRUCTS_SETTINGSTYPE_H

#include "../../ESPEasy_common.h"


class SettingsType {
public:

  enum class Enum {
    BasicSettings_Type = 0,
    TaskSettings_Type,
    CustomTaskSettings_Type,
    ControllerSettings_Type,
    CustomControllerSettings_Type,
    NotificationSettings_Type,
    SecuritySettings_Type,
    ExtdControllerCredentials_Type,

    SettingsType_MAX
  };

  enum class SettingsFileEnum {
    FILE_CONFIG_type,
    FILE_NOTIFICATION_type,
    FILE_SECURITY_type,
    FILE_UNKNOWN_type
  };

  static String getSettingsTypeString(Enum settingsType);
  static bool   getSettingsParameters(Enum settingsType,
                                      int  index,
                                      int& offset,
                                      int& max_size);
  static bool getSettingsParameters(Enum settingsType,
                                    int  index,
                                    int& max_index,
                                    int& offset,
                                    int& max_size,
                                    int& struct_size);

  static int              getMaxFilePos(Enum settingsType);
  static int              getFileSize(Enum settingsType);

#ifndef BUILD_MINIMAL_OTA
  static unsigned int     getSVGcolor(Enum settingsType);
#endif // ifndef BUILD_MINIMAL_OTA

  static SettingsFileEnum getSettingsFile(Enum settingsType);
  static String           getSettingsFileName(Enum settingsType);
};


#endif // DATASTRUCTS_SETTINGSTYPE_H
