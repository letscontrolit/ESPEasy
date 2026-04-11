#ifndef _SRC_HELPERS_I2C_PLUGIN_HELPER_H
#define _SRC_HELPERS_I2C_PLUGIN_HELPER_H


#include "../../ESPEasy_common.h"

#if FEATURE_I2C

#include "../Globals/Plugins.h"
#include "../Globals/Settings.h"
#include "../Helpers/Hardware_I2C.h"

#include "../WebServer/HTML_wrappers.h"

bool checkI2CConfigValid_toHtml(taskIndex_t taskIndex,
                                bool        outputToHtml = true);

#endif
#endif // ifndef _SRC_HELPERS_I2C_PLUGIN_HELPER_H
