#include "../Globals/SecuritySettings.h"


SecurityStruct SecuritySettings;
ExtendedControllerCredentialsStruct ExtendedControllerCredentials;

#if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE
SecurityStruct_deviceSpecific SecuritySettings_deviceSpecific;
#endif
