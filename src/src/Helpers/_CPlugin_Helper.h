#ifndef CPLUGIN_HELPER_H
#define CPLUGIN_HELPER_H

#include "../../ESPEasy_common.h"
#include "../../_Plugin_Helper.h"


#include "../ControllerQueue/DelayQueueElements.h" // Also forward declaring the do_process_cNNN_delay_queue
#include "../DataStructs/ControllerSettingsStruct.h"
#include "../ESPEasyCore/Controller.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../Globals/CPlugins.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/Services.h"
#include "../Helpers/_CPlugin_init.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Network.h"
#include "../Helpers/Networking.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/_CPlugin_Helper_webform.h"



/*********************************************************************************************\
* Helper functions used in a number of controllers
\*********************************************************************************************/
bool safeReadStringUntil(Stream     & input,
                         String     & str,
                         char         terminator,
                         unsigned int maxSize = 1024,
                         unsigned int timeout = 1000);


#ifndef BUILD_NO_DEBUG
void log_connecting_to(const __FlashStringHelper * prefix, int controller_number, ControllerSettingsStruct& ControllerSettings);
#endif // ifndef BUILD_NO_DEBUG

void log_connecting_fail(const __FlashStringHelper * prefix, int controller_number);

bool count_connection_results(bool success, const __FlashStringHelper * prefix, int controller_number, unsigned long connect_start_time);

#if FEATURE_HTTP_CLIENT
bool try_connect_host(int controller_number, WiFiUDP& client, ControllerSettingsStruct& ControllerSettings);

bool try_connect_host(int controller_number, WiFiClient& client, ControllerSettingsStruct& ControllerSettings);

bool try_connect_host(int controller_number, WiFiClient& client, ControllerSettingsStruct& ControllerSettings, const __FlashStringHelper * loglabel);

// Use "client.available() || client.connected()" to read all lines from slow servers.
// See: https://github.com/esp8266/Arduino/pull/5113
//      https://github.com/esp8266/Arduino/pull/1829
bool client_available(WiFiClient& client);



String send_via_http(int                             controller_number,
                     const ControllerSettingsStruct& ControllerSettings,
                     controllerIndex_t               controller_idx,
                     const String                  & uri,
                     const String                  & HttpMethod,
                     const String                  & header,
                     const String                  & postStr,
                     int                           & httpCode);
#endif // FEATURE_HTTP_CLIENT
                     

String getControllerUser(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings, bool parseTemplate = true);
String getControllerPass(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings);
void setControllerUser(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings, const String& value);
void setControllerPass(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings, const String& value);

bool hasControllerCredentialsSet(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings);


#endif // CPLUGIN_HELPER_H
