#pragma once

#include "../../../ESPEasy_common.h"

#if FEATURE_WIFI

# include "../wifi/ESPEasyWifi.h"
# include "../wifi/ESPEasyWiFi_state_machine.h"

extern ESPEasy::net::wifi::ESPEasyWiFi_t ESPEasyWiFi;
#endif // if FEATURE_WIFI
