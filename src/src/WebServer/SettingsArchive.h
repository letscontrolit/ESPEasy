#pragma once

#include "../WebServer/common.h"

#if FEATURE_SETTINGS_ARCHIVE

# include "../DataTypes/ESPEasyFileType.h"

# if FEATURE_CUSTOM_PROVISIONING
#  include "../DataStructs/ProvisioningStruct.h"
# endif


// ********************************************************************************
// Web Interface to manage archived settings
// ********************************************************************************
void handle_settingsarchive();

#endif // if FEATURE_SETTINGS_ARCHIVE
