#ifndef GLOBALS_SECURITY_SETTINGS_H
#define GLOBALS_SECURITY_SETTINGS_H

#include "../DataStructs/SecurityStruct.h"
#include "../DataStructs/ExtendedControllerCredentialsStruct.h"
#if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE
#include "../DataStructs/SecurityStruct_deviceSpecific.h"
#endif

extern SecurityStruct SecuritySettings;
extern ExtendedControllerCredentialsStruct ExtendedControllerCredentials;
#if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE
extern SecurityStruct_deviceSpecific SecuritySettings_deviceSpecific;
#endif

#endif // GLOBALS_SECURITY_SETTINGS_H