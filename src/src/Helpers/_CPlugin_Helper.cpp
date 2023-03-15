#include "../Helpers/_CPlugin_Helper.h"

#include "../../ESPEasy_common.h"

#include "../CustomBuild/CompiletimeDefines.h"
#include "../CustomBuild/ESPEasyLimits.h"

#include "../DataStructs/SecurityStruct.h"
#include "../DataStructs/SettingsStruct.h"

#include "../DataStructs/ControllerSettingsStruct.h"
#include "../DataStructs/TimingStats.h"

#include "../ESPEasyCore/ESPEasy_backgroundtasks.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyEth.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"

#include "../Globals/Settings.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/ESPEasyWiFiEvent.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Network.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringConverter.h"

#include <WiFiClient.h>
#include <WiFiUdp.h>


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

  str = String();

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

#ifndef BUILD_NO_DEBUG
void log_connecting_to(const __FlashStringHelper *prefix, int controller_number, ControllerSettingsStruct& ControllerSettings) {
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = prefix;
    log += get_formatted_Controller_number(controller_number);
    log += F(" connecting to ");
    log += ControllerSettings.getHostPortString();
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
}

#endif // ifndef BUILD_NO_DEBUG

void log_connecting_fail(const __FlashStringHelper *prefix, int controller_number) {
  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
    String log = prefix;
    log += get_formatted_Controller_number(controller_number);
    log += F(" connection failed (");
    log += WiFiEventData.connectionFailures;
    log += F("/");
    log += Settings.ConnectionFailuresThreshold;
    log += F(")");
    addLogMove(LOG_LEVEL_ERROR, log);
  }
}

bool count_connection_results(bool success, const __FlashStringHelper *prefix, int controller_number, unsigned long connect_start_time) {
  WiFiEventData.connectDurations[controller_number] = timePassedSince(connect_start_time);
  if (!success)
  {
    ++WiFiEventData.connectionFailures;
    log_connecting_fail(prefix, controller_number);
    return false;
  }
  statusLED(true);

  if (WiFiEventData.connectionFailures > 0) {
    --WiFiEventData.connectionFailures;
  }
  return true;
}

bool try_connect_host(int controller_number, WiFiUDP& client, ControllerSettingsStruct& ControllerSettings) {
  START_TIMER;

  if (!NetworkConnected()) { 
    client.stop();
    return false; 
  }
  // Ignoring the ACK from the server is probably set for a reason.
  // For example because the server does not give an acknowledgement.
  // This way, we always need the set amount of timeout to handle the request.
  // Thus we should not make the timeout dynamic here if set to ignore ack.
  const uint32_t timeout = ControllerSettings.MustCheckReply 
    ? WiFiEventData.getSuggestedTimeout(controller_number, ControllerSettings.ClientTimeout)
    : ControllerSettings.ClientTimeout;

  client.setTimeout(timeout); // in msec as it should be!
  delay(0);
#ifndef BUILD_NO_DEBUG
  log_connecting_to(F("UDP  : "), controller_number, ControllerSettings);
#endif // ifndef BUILD_NO_DEBUG

  const unsigned long connect_start_time = millis();  
  bool success      = ControllerSettings.beginPacket(client);
  if (!success) {
    client.stop();
  }
  const bool result = count_connection_results(
    success,
    F("UDP  : "), 
    controller_number,
    connect_start_time);
  STOP_TIMER(TRY_CONNECT_HOST_UDP);
  return result;
}

#if FEATURE_HTTP_CLIENT
bool try_connect_host(int controller_number, WiFiClient& client, ControllerSettingsStruct& ControllerSettings) {
  return try_connect_host(controller_number, client, ControllerSettings, F("HTTP : "));
}

bool try_connect_host(int                        controller_number,
                      WiFiClient               & client,
                      ControllerSettingsStruct & ControllerSettings,
                      const __FlashStringHelper *loglabel) {
  START_TIMER;

  if (!NetworkConnected()) { 
    client.stop();
    return false;
  }

  // Use WiFiClient class to create TCP connections
  delay(0);

  // Ignoring the ACK from the server is probably set for a reason.
  // For example because the server does not give an acknowledgement.
  // This way, we always need the set amount of timeout to handle the request.
  // Thus we should not make the timeout dynamic here if set to ignore ack.
  const uint32_t timeout = ControllerSettings.MustCheckReply 
    ? WiFiEventData.getSuggestedTimeout(controller_number, ControllerSettings.ClientTimeout)
    : ControllerSettings.ClientTimeout;

  #ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

  // See: https://github.com/espressif/arduino-esp32/pull/6676
  client.setTimeout((timeout + 500) / 1000); // in seconds!!!!
  Client *pClient = &client;
  pClient->setTimeout(timeout);
  #else // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS
  client.setTimeout(timeout);                // in msec as it should be!
  #endif // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

#ifndef BUILD_NO_DEBUG
  log_connecting_to(loglabel, controller_number, ControllerSettings);
#endif // ifndef BUILD_NO_DEBUG

  const unsigned long connect_start_time = millis();

  const bool success = ControllerSettings.connectToHost(client);
  if (!success) {
    client.stop();
  }
  const bool result  = count_connection_results(
    success,
    loglabel, 
    controller_number,
    connect_start_time);
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

String send_via_http(int                             controller_number,
                     const ControllerSettingsStruct& ControllerSettings,
                     controllerIndex_t               controller_idx,
                     const String                  & uri,
                     const String                  & HttpMethod,
                     const String                  & header,
                     const String                  & postStr,
                     int                           & httpCode) {

  // Ignoring the ACK from the HTTP server is probably set for a reason.
  // For example because the server does not give an acknowledgement.
  // This way, we always need the set amount of timeout to handle the request.
  // Thus we should not make the timeout dynamic here if set to ignore ack.
  const uint32_t timeout = ControllerSettings.MustCheckReply 
    ? WiFiEventData.getSuggestedTimeout(controller_number, ControllerSettings.ClientTimeout)
    : ControllerSettings.ClientTimeout;

  const unsigned long connect_start_time = millis();
  const String result = send_via_http(
    get_formatted_Controller_number(controller_number),
    timeout,
    getControllerUser(controller_idx, ControllerSettings),
    getControllerPass(controller_idx, ControllerSettings),
    ControllerSettings.getHost(),
    ControllerSettings.Port,
    uri,
    HttpMethod,
    header,
    postStr,
    httpCode,
    ControllerSettings.MustCheckReply);

  // FIXME TD-er: Shouldn't this be: success = (httpCode >= 100) && (httpCode < 300)
  // or is reachability of the host the important factor here?
  const bool success = httpCode > 0;

  count_connection_results(
    success,
    F("HTTP  : "),
    controller_number,
    connect_start_time);

  return result;
}
#endif // FEATURE_HTTP_CLIENT





String getControllerUser(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings, bool doParseTemplate)
{
  if (!validControllerIndex(controller_idx)) { return EMPTY_STRING; }

  String res;
  if (ControllerSettings.useExtendedCredentials()) {
    res = ExtendedControllerCredentials.getControllerUser(controller_idx);
  } else {
    res = String(SecuritySettings.ControllerUser[controller_idx]);
  }
  res.trim();
  if (doParseTemplate) {
    res = parseTemplate(res);
  }
  return res;
}

String getControllerPass(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings)
{
  if (!validControllerIndex(controller_idx)) { return EMPTY_STRING; }

  if (ControllerSettings.useExtendedCredentials()) {
    return ExtendedControllerCredentials.getControllerPass(controller_idx);
  }
  String res(SecuritySettings.ControllerPassword[controller_idx]);
  res.trim();
  return res;
}

void setControllerUser(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings, const String& value)
{
  if (!validControllerIndex(controller_idx)) { return; }

  if (ControllerSettings.useExtendedCredentials()) {
    ExtendedControllerCredentials.setControllerUser(controller_idx, value);
  } else {
    safe_strncpy(SecuritySettings.ControllerUser[controller_idx], value, sizeof(SecuritySettings.ControllerUser[0]));
  }
}

void setControllerPass(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings, const String& value)
{
  if (!validControllerIndex(controller_idx)) { return; }

  if (ControllerSettings.useExtendedCredentials()) {
    ExtendedControllerCredentials.setControllerPass(controller_idx, value);
  } else {
    safe_strncpy(SecuritySettings.ControllerPassword[controller_idx], value, sizeof(SecuritySettings.ControllerPassword[0]));
  }
}

bool hasControllerCredentialsSet(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings)
{
  return !getControllerUser(controller_idx, ControllerSettings, false).isEmpty() &&
         !getControllerPass(controller_idx, ControllerSettings).isEmpty();
}
