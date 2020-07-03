#include "_CPlugin_Helper.h"

#include "ESPEasy_fdwdecl.h"
#include "ESPEasy_Log.h"
#include "ESPEasy_common.h"

#include "src/DataStructs/SecurityStruct.h"
#include "src/DataStructs/SettingsStruct.h"

#include "src/DataStructs/ControllerSettingsStruct.h"
#include "src/DataStructs/ESPEasyLimits.h"
#include "src/DataStructs/TimingStats.h"

#include "src/Globals/Settings.h"
#include "src/Globals/SecuritySettings.h"
#include "src/Globals/ESPEasyWiFiEvent.h"

#include "src/Helpers/CompiletimeDefines.h"
#include "src/Helpers/ESPEasy_time_calc.h"

#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <base64.h>

bool safeReadStringUntil(Stream     & input,
                         String     & str,
                         char         terminator,
                         unsigned int maxSize,
                         unsigned int timeout)
{
  int c;
  const unsigned long start           = millis();
  const unsigned long timer           = start + timeout;
  unsigned long backgroundtasks_timer = start + 10;

  str = "";

  do {
    // read character
    if (input.available()) {
      c = input.read();

      if (c >= 0) {
        // found terminator, we're ok
        if (c == terminator) {
          return true;
        }

        // found character, add to string
        str += char(c);

        // string at max size?
        if (str.length() >= maxSize) {
          addLog(LOG_LEVEL_ERROR, F("Not enough bufferspace to read all input data!"));
          return false;
        }
      }

      // We must run the backgroundtasks every now and then.
      if (timeOutReached(backgroundtasks_timer)) {
        backgroundtasks_timer += 10;
        backgroundtasks();
      } else {
        delay(0);
      }
    } else {
      delay(0);
    }
  } while (!timeOutReached(timer));

  addLog(LOG_LEVEL_ERROR, F("Timeout while reading input data!"));
  return false;
}

String get_formatted_Controller_number(cpluginID_t cpluginID) {
  if (!validCPluginID(cpluginID)) {
    return F("C---");
  }
  String result = F("C");

  if (cpluginID < 100) { result += '0'; }

  if (cpluginID < 10) { result += '0'; }
  result += cpluginID;
  return result;
}

String get_auth_header(const String& user, const String& pass) {
  String authHeader = "";

  if ((user.length() != 0) && (pass.length() != 0)) {
    String auth = user;
    auth       += ":";
    auth       += pass;
    authHeader  = F("Authorization: Basic ");
    authHeader += base64::encode(auth);
    authHeader += F(" \r\n");
  }
  return authHeader;
}

String get_auth_header(int controller_index, const ControllerSettingsStruct& ControllerSettings) {
  String authHeader = "";

  if (validControllerIndex(controller_index)) {
    if (hasControllerCredentialsSet(controller_index, ControllerSettings))
    {
      authHeader = get_auth_header(
        getControllerUser(controller_index, ControllerSettings),
        getControllerPass(controller_index, ControllerSettings));
    }
  } else {
    addLog(LOG_LEVEL_ERROR, F("Invalid controller index"));
  }
  return authHeader;
}

String get_user_agent_request_header_field() {
  static unsigned int agent_size = 20;
  String request;

  request.reserve(agent_size);
  request    = F("User-Agent: ");
  request   += F("ESP Easy/");
  request   += BUILD;
  request   += '/';
  request   += get_build_date();
  request   += ' ';
  request   += get_build_time();
  request   += "\r\n";
  agent_size = request.length();
  return request;
}

String do_create_http_request(
  const String& hostportString,
  const String& method, const String& uri,
  const String& auth_header, const String& additional_options,
  int content_length) {
  int estimated_size = hostportString.length() + method.length()
                       + uri.length() + auth_header.length()
                       + additional_options.length()
                       + 42;

  if (content_length >= 0) { estimated_size += 45; }
  String request;
  request.reserve(estimated_size);
  request += method;
  request += ' ';

  if (!uri.startsWith("/")) { request += '/'; }
  request += uri;
  request += F(" HTTP/1.1");
  request += "\r\n";

  if (content_length >= 0) {
    request += F("Content-Length: ");
    request += content_length;
    request += "\r\n";
  }
  request += F("Host: ");
  request += hostportString;
  request += "\r\n";
  request += auth_header;

  // Add request header as fall back.
  // When adding another "accept" header, it may be interpreted as:
  // "if you have XXX, send it; or failing that, just give me what you've got."
  request += F("Accept: */*;q=0.1");
  request += "\r\n";
  request += additional_options;
  request += get_user_agent_request_header_field();
  request += F("Connection: close\r\n");
  request += "\r\n";
#ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG, request);
#endif // ifndef BUILD_NO_DEBUG
  return request;
}

String do_create_http_request(
  const String& hostportString,
  const String& method, const String& uri) {
  return do_create_http_request(hostportString, method, uri,
                                "", // auth_header
                                "", // additional_options
                                -1  // content_length
                                );
}

String do_create_http_request(
  int controller_number, ControllerSettingsStruct& ControllerSettings,
  const String& method, const String& uri,
  int content_length) {
  const bool defaultport = ControllerSettings.Port == 0 || ControllerSettings.Port == 80;

  return do_create_http_request(
    defaultport ? ControllerSettings.getHost() : ControllerSettings.getHostPortString(),
    method,
    uri,
    "", // auth_header
    "", // additional_options
    content_length);
}

String create_http_request_auth(
  int controller_number, int controller_index, ControllerSettingsStruct& ControllerSettings,
  const String& method, const String& uri,
  int content_length) {
  const bool defaultport = ControllerSettings.Port == 0 || ControllerSettings.Port == 80;

  return do_create_http_request(
    defaultport ? ControllerSettings.getHost() : ControllerSettings.getHostPortString(),
    method,
    uri,
    get_auth_header(controller_index, ControllerSettings),
    "", // additional_options
    content_length);
}

String create_http_get_request(int controller_number, ControllerSettingsStruct& ControllerSettings,
                               const String& uri) {
  return do_create_http_request(controller_number, ControllerSettings, F("GET"), uri, -1);
}

String create_http_request_auth(int controller_number, int controller_index, ControllerSettingsStruct& ControllerSettings,
                                const String& method, const String& uri) {
  return create_http_request_auth(controller_number, controller_index, ControllerSettings, method, uri, -1);
}

#ifndef BUILD_NO_DEBUG
void log_connecting_to(const String& prefix, int controller_number, ControllerSettingsStruct& ControllerSettings) {
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = prefix;
    log += get_formatted_Controller_number(controller_number);
    log += F(" connecting to ");
    log += ControllerSettings.getHostPortString();
    addLog(LOG_LEVEL_DEBUG, log);
  }
}

#endif // ifndef BUILD_NO_DEBUG

void log_connecting_fail(const String& prefix, int controller_number, ControllerSettingsStruct& ControllerSettings) {
  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
    String log = prefix;
    log += get_formatted_Controller_number(controller_number);
    log += F(" connection failed (");
    log += connectionFailures;
    log += F("/");
    log += Settings.ConnectionFailuresThreshold;
    log += F(")");
    addLog(LOG_LEVEL_ERROR, log);
  }
}

bool count_connection_results(bool success, const String& prefix, int controller_number, ControllerSettingsStruct& ControllerSettings) {
  if (!success)
  {
    ++connectionFailures;
    log_connecting_fail(prefix, controller_number, ControllerSettings);
    return false;
  }
  statusLED(true);

  if (connectionFailures > 0) {
    --connectionFailures;
  }
  return true;
}

bool try_connect_host(int controller_number, WiFiUDP& client, ControllerSettingsStruct& ControllerSettings) {
  START_TIMER;

  if (!NetworkConnected()) { return false; }
  client.setTimeout(ControllerSettings.ClientTimeout);
#ifndef BUILD_NO_DEBUG
  log_connecting_to(F("UDP  : "), controller_number, ControllerSettings);
#endif // ifndef BUILD_NO_DEBUG
  bool success      = ControllerSettings.beginPacket(client);
  const bool result = count_connection_results(
    success,
    F("UDP  : "), controller_number, ControllerSettings);
  STOP_TIMER(TRY_CONNECT_HOST_UDP);
  return result;
}

bool try_connect_host(int controller_number, WiFiClient& client, ControllerSettingsStruct& ControllerSettings) {
  return try_connect_host(controller_number, client, ControllerSettings, F("HTTP : "));	
}	

bool try_connect_host(int controller_number, WiFiClient& client, ControllerSettingsStruct& ControllerSettings, const String& loglabel) {
  START_TIMER;

  if (!NetworkConnected()) { return false; }

  // Use WiFiClient class to create TCP connections
  client.setTimeout(ControllerSettings.ClientTimeout);
#ifndef BUILD_NO_DEBUG
  log_connecting_to(loglabel, controller_number, ControllerSettings);
#endif // ifndef BUILD_NO_DEBUG
  const bool success = ControllerSettings.connectToHost(client);
  const bool result  = count_connection_results(
    success,
    loglabel, controller_number, ControllerSettings);
  STOP_TIMER(TRY_CONNECT_HOST_TCP);
  return result;
}

// Use "client.available() || client.connected()" to read all lines from slow servers.
// See: https://github.com/esp8266/Arduino/pull/5113
//      https://github.com/esp8266/Arduino/pull/1829
bool client_available(WiFiClient& client) {
  delay(0);
  return (client.available() != 0) || (client.connected() != 0);
}

bool send_via_http(const String& logIdentifier, WiFiClient& client, const String& postStr, bool must_check_reply) {
  bool success = !must_check_reply;

  // This will send the request to the server
  byte written = client.print(postStr);

  // as of 2018/11/01 the print function only returns one byte (upd to 256 chars sent). However if the string sent can be longer than this
  // therefore we calculate modulo 256.
  // see discussion here https://github.com/letscontrolit/ESPEasy/pull/1979
  // and implementation here
  // https://github.com/esp8266/Arduino/blob/561426c0c77e9d05708f2c4bf2a956d3552a3706/libraries/ESP8266WiFi/src/include/ClientContext.h#L437-L467
  // this needs to be adjusted if the WiFiClient.print method changes.
  if (written != (postStr.length() % 256)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("HTTP : ");
      log += logIdentifier;
      log += F(" Error: could not write to client (");
      log += written;
      log += "/";
      log += postStr.length();
      log += ")";
      addLog(LOG_LEVEL_ERROR, log);
    }
    success = false;
  }
#ifndef BUILD_NO_DEBUG
  else {
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("HTTP : ");
      log += logIdentifier;
      log += F(" written to client (");
      log += written;
      log += "/";
      log += postStr.length();
      log += ")";
      addLog(LOG_LEVEL_DEBUG, log);
    }
  }
#endif // ifndef BUILD_NO_DEBUG

  if (must_check_reply) {
    unsigned long timer = millis() + 200;

    while (!client_available(client)) {
      if (timeOutReached(timer)) { return false; }
      delay(1);
    }

    // Read all the lines of the reply from server and print them to Serial
    while (client_available(client) && !success) {
      //   String line = client.readStringUntil('\n');
      String line;
      safeReadStringUntil(client, line, '\n');

#ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
        if (line.length() > 80) {
          addLog(LOG_LEVEL_DEBUG_MORE, line.substring(0, 80));
        } else {
          addLog(LOG_LEVEL_DEBUG_MORE, line);
        }
      }
#endif // ifndef BUILD_NO_DEBUG

      if (line.startsWith(F("HTTP/1.1 2")))
      {
        success = true;

        // Leave this debug info in the build, regardless of the
        // BUILD_NO_DEBUG flags.
        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("HTTP : ");
          log += logIdentifier;
          log += F(" Success! ");
          log += line;
          addLog(LOG_LEVEL_DEBUG, log);
        }
      } else if (line.startsWith(F("HTTP/1.1 4"))) {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log = F("HTTP : ");
          log += logIdentifier;
          log += F(" Error: ");
          log += line;
          addLog(LOG_LEVEL_ERROR, log);
        }
#ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG_MORE, postStr);
#endif // ifndef BUILD_NO_DEBUG
      }
      delay(0);
    }
  }
#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("HTTP : ");
    log += logIdentifier;
    log += F(" closing connection");
    addLog(LOG_LEVEL_DEBUG, log);
  }
#endif // ifndef BUILD_NO_DEBUG

  client.flush();
  client.stop();
  return success;
}

bool send_via_http(int controller_number, WiFiClient& client, const String& postStr, bool must_check_reply) {
  return send_via_http(get_formatted_Controller_number(controller_number), client, postStr, must_check_reply);
}


String getControllerUser(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings)
{
  if (!validControllerIndex(controller_idx)) return "";
  if (ControllerSettings.useExtendedCredentials()) {
    return ExtendedControllerCredentials.getControllerUser(controller_idx);
  }
  return SecuritySettings.ControllerUser[controller_idx];
}

String getControllerPass(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings)
{
  if (!validControllerIndex(controller_idx)) return "";
  if (ControllerSettings.useExtendedCredentials()) {
    return ExtendedControllerCredentials.getControllerPass(controller_idx);
  }
  return SecuritySettings.ControllerPassword[controller_idx];
}

void setControllerUser(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings, const String& value)
{
  if (!validControllerIndex(controller_idx)) return;
  if (ControllerSettings.useExtendedCredentials()) {
    ExtendedControllerCredentials.setControllerUser(controller_idx, value);
  } else {
    safe_strncpy(SecuritySettings.ControllerUser[controller_idx], value, sizeof(SecuritySettings.ControllerUser[0]));
  }
}

void setControllerPass(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings, const String& value)
{
  if (!validControllerIndex(controller_idx)) return;
  if (ControllerSettings.useExtendedCredentials()) {
    ExtendedControllerCredentials.setControllerPass(controller_idx, value);
  } else {
    safe_strncpy(SecuritySettings.ControllerPassword[controller_idx], value, sizeof(SecuritySettings.ControllerPassword[0]));
  }
}

bool hasControllerCredentialsSet(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings)
{
  return getControllerUser(controller_idx, ControllerSettings).length() != 0 &&
         getControllerPass(controller_idx, ControllerSettings).length() != 0;
}