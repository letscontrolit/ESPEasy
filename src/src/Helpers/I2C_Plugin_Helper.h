#ifndef _SRC_HELPERS_I2C_PLUGIN_HELPER_H
#define _SRC_HELPERS_I2C_PLUGIN_HELPER_H

#include <Arduino.h>

#include "../../ESPEasy_common.h"

#include "../Globals/Plugins.h"
#include "../Globals/Settings.h"

#include "../WebServer/HTML_wrappers.h"

bool checkI2CConfigValid_toHtml(taskIndex_t taskIndex,
                                bool        outputToHtml = true);

#endif // ifndef _SRC_HELPERS_I2C_PLUGIN_HELPER_H
