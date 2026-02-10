#pragma once

#include "../DataStructs/SettingsStruct.h"
#include "../DataTypes/TaskIndex.h"
#include "../Globals/Plugins.h"
#include "../Globals/Settings.h"
#include "../Helpers/Hardware_device_info.h"

#include "../WebServer/Markup_Forms.h"

void SPIInterfaceSelector(String  label,
                          String  id,
                          uint8_t choice,
                          bool    disabled = false);
