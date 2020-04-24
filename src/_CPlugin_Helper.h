#ifndef CPLUGIN_HELPER_H
#define CPLUGIN_HELPER_H CPLUGIN_HELPER_H

#include <Arduino.h>
#include "src/Globals/CPlugins.h"

struct ControllerSettingsStruct;
class WiFiUDP;
class WiFiClient;

/*********************************************************************************************\
* Helper functions used in a number of controllers
\*********************************************************************************************/
bool safeReadStringUntil(Stream     & input,
                         String     & str,
                         char         terminator,
                         unsigned int maxSize = 1024,
                         unsigned int timeout = 1000);

String get_formatted_Controller_number(cpluginID_t cpluginID);

String get_auth_header(const String& user, const String& pass);

String get_auth_header(int controller_index, const ControllerSettingsStruct& ControllerSettings);

String get_user_agent_request_header_field();

String do_create_http_request(
  const String& hostportString,
  const String& method, const String& uri,
  const String& auth_header, const String& additional_options,
  int content_length);

String do_create_http_request(
  const String& hostportString,
  const String& method, const String& uri);

String do_create_http_request(
  int controller_number, ControllerSettingsStruct& ControllerSettings,
  const String& method, const String& uri,
  int content_length);

String create_http_request_auth(
  int controller_number, int controller_index, ControllerSettingsStruct& ControllerSettings,
  const String& method, const String& uri,
  int content_length);

String create_http_get_request(int controller_number, ControllerSettingsStruct& ControllerSettings,
                               const String& uri);

String create_http_request_auth(int controller_number, int controller_index, ControllerSettingsStruct& ControllerSettings,
                                const String& method, const String& uri);

#ifndef BUILD_NO_DEBUG
void log_connecting_to(const String& prefix, int controller_number, ControllerSettingsStruct& ControllerSettings);
#endif // ifndef BUILD_NO_DEBUG

void log_connecting_fail(const String& prefix, int controller_number, ControllerSettingsStruct& ControllerSettings);

bool count_connection_results(bool success, const String& prefix, int controller_number, ControllerSettingsStruct& ControllerSettings);

bool try_connect_host(int controller_number, WiFiUDP& client, ControllerSettingsStruct& ControllerSettings);

bool try_connect_host(int controller_number, WiFiClient& client, ControllerSettingsStruct& ControllerSettings);

bool try_connect_host(int controller_number, WiFiClient& client, ControllerSettingsStruct& ControllerSettings, const String& loglabel);

// Use "client.available() || client.connected()" to read all lines from slow servers.
// See: https://github.com/esp8266/Arduino/pull/5113
//      https://github.com/esp8266/Arduino/pull/1829
bool client_available(WiFiClient& client);

bool send_via_http(const String& logIdentifier, WiFiClient& client, const String& postStr, bool must_check_reply);

bool send_via_http(int controller_number, WiFiClient& client, const String& postStr, bool must_check_reply);

String getControllerUser(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings);
String getControllerPass(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings);
void setControllerUser(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings, const String& value);
void setControllerPass(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings, const String& value);

bool hasControllerCredentialsSet(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings);


#endif // CPLUGIN_HELPER_H
