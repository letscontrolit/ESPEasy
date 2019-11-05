#ifndef DATASTRUCTS_SETTINGSTYPE_H
#define DATASTRUCTS_SETTINGSTYPE_H

#include "../../ESPEasy_common.h"

enum SettingsType {
  BasicSettings_Type = 0,
  TaskSettings_Type,
  CustomTaskSettings_Type,
  ControllerSettings_Type,
  CustomControllerSettings_Type,
  NotificationSettings_Type,

  SettingsType_MAX

};
String getSettingsTypeString(SettingsType settingsType);
bool getSettingsParameters(SettingsType settingsType, int index, int& offset, int& max_size);
bool getSettingsParameters(SettingsType settingsType, int index, int& max_index, int& offset, int& max_size, int& struct_size);


#endif // DATASTRUCTS_SETTINGSTYPE_H