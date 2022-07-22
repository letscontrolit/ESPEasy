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
#include <base64.h>
#include <MD5Builder.h>


#ifdef ESP8266
# include <ESP8266HTTPClient.h>
#endif // ifdef ESP8266
#ifdef ESP32
# include <HTTPClient.h>
#endif // ifdef ESP32

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

String get_user_agent_string() {
  static unsigned int agent_size = 20;
  String userAgent;

  userAgent.reserve(agent_size);
  userAgent += F("ESP Easy/");
  userAgent += BUILD;
  userAgent += '/';
  userAgent += get_build_date();
  userAgent += ' ';
  userAgent += get_build_time();
  agent_size = userAgent.length();
  return userAgent;
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

bool count_connection_results(bool success, const __FlashStringHelper *prefix, int controller_number) {
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

  if (!NetworkConnected()) { return false; }
  #ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS
  // See: https://github.com/espressif/arduino-esp32/pull/6676
  client.setTimeout((ControllerSettings.ClientTimeout + 500) / 1000); // in seconds!!!!
  #else
  client.setTimeout(ControllerSettings.ClientTimeout); // in msec as it should be!  
  #endif
  delay(0);
#ifndef BUILD_NO_DEBUG
  log_connecting_to(F("UDP  : "), controller_number, ControllerSettings);
#endif // ifndef BUILD_NO_DEBUG
  bool success      = ControllerSettings.beginPacket(client);
  const bool result = count_connection_results(
    success,
    F("UDP  : "), controller_number);
  STOP_TIMER(TRY_CONNECT_HOST_UDP);
  return result;
}

bool try_connect_host(int controller_number, WiFiClient& client, ControllerSettingsStruct& ControllerSettings) {
  return try_connect_host(controller_number, client, ControllerSettings, F("HTTP : "));
}

bool try_connect_host(int                        controller_number,
                      WiFiClient               & client,
                      ControllerSettingsStruct & ControllerSettings,
                      const __FlashStringHelper *loglabel) {
  START_TIMER;

  if (!NetworkConnected()) { return false; }

  // Use WiFiClient class to create TCP connections
  delay(0);
  #ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS
  // See: https://github.com/espressif/arduino-esp32/pull/6676
  client.setTimeout((ControllerSettings.ClientTimeout + 500) / 1000); // in seconds!!!!
  #else
  client.setTimeout(ControllerSettings.ClientTimeout); // in msec as it should be!  
  #endif

#ifndef BUILD_NO_DEBUG
  log_connecting_to(loglabel, controller_number, ControllerSettings);
#endif // ifndef BUILD_NO_DEBUG
  const bool success = ControllerSettings.connectToHost(client);
  const bool result  = count_connection_results(
    success,
    loglabel, controller_number);
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
                     WiFiClient                    & client,
                     const String                  & uri,
                     const String                  & HttpMethod,
                     const String                  & header,
                     const String                  & postStr,
                     int                           & httpCode) {
  const String result = send_via_http(
    get_formatted_Controller_number(controller_number),
    client,
    ControllerSettings.ClientTimeout,
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
    controller_number);

  return result;
}

bool splitHeaders(int& strpos, const String& multiHeaders, String& name, String& value) {
  if (strpos < 0) {
    return false;
  }
  int colonPos = multiHeaders.indexOf(':', strpos);

  if (colonPos < 0) {
    return false;
  }
  name = multiHeaders.substring(strpos, colonPos);
  int valueEndPos = multiHeaders.indexOf('\n', colonPos + 1);

  if (valueEndPos < 0) {
    value  = multiHeaders.substring(colonPos + 1);
    strpos = -1;
  } else {
    value  = multiHeaders.substring(colonPos + 1, valueEndPos);
    strpos = valueEndPos + 1;
  }
  value.replace('\r', ' ');
  value.trim();
  return true;
}

String extractParam(const String& authReq, const String& param, const char delimit) {
  int _begin = authReq.indexOf(param);

  if (_begin == -1) { return EMPTY_STRING; }
  return authReq.substring(_begin + param.length(), authReq.indexOf(delimit, _begin + param.length()));
}

String getCNonce(const int len) {
  static const char alphanum[] = "0123456789"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz";
  String s;

  for (int i = 0; i < len; ++i) {
    s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return s;
}

String getDigestAuth(const String& authReq,
                     const String& username,
                     const String& password,
                     const String& method,
                     const String& uri,
                     unsigned int  counter) {
  // extracting required parameters for RFC 2069 simpler Digest
  const String realm  = extractParam(authReq, F("realm=\""), '"');
  const String nonce  = extractParam(authReq, F("nonce=\""), '"');
  const String cNonce = getCNonce(8);

  char nc[9];

  snprintf(nc, sizeof(nc), "%08x", counter);

  // parameters for the RFC 2617 newer Digest
  MD5Builder md5;

  md5.begin();
  md5.add(username + ':' + realm + ':' + password); // md5 of the user:realm:user
  md5.calculate();
  const String h1 = md5.toString();

  md5.begin();
  md5.add(method + ':' + uri);
  md5.calculate();
  const String h2 = md5.toString();

  md5.begin();
  md5.add(h1 + ':' + nonce + ':' + String(nc) + ':' + cNonce + F(":auth:") + h2);
  md5.calculate();
  const String response = md5.toString();

  const String authorization =
    String(F("Digest username=\"")) + username +
    F("\", realm=\"") + realm +
    F("\", nonce=\"") + nonce +
    F("\", uri=\"") + uri +
    F("\", algorithm=\"MD5\", qop=auth, nc=") + String(nc) +
    F(", cnonce=\"") + cNonce +
    F("\", response=\"") + response +
    '"';

  //  Serial.println(authorization);

  return authorization;
}

void log_http_result(const HTTPClient& http,
                     const String    & logIdentifier,
                     const String    & HttpMethod,
                     int               httpCode,
                     const String    & response)
{
  uint8_t loglevel = LOG_LEVEL_ERROR;
  bool    success  = false;

  // HTTP codes:
  // 1xx Informational response
  // 2xx Success
  if ((httpCode >= 100) && (httpCode < 300)) {
    loglevel = LOG_LEVEL_INFO;
    success  = true;
  }

  if (loglevelActiveFor(loglevel)) {
    String log = F("HTTP : ");
    log += logIdentifier;
    log += ' ';
    log += HttpMethod;
    log += F("... ");

    if (!success) {
      log += F("failed ");
    }
    log += F("HTTP code: ");
    log += String(httpCode);

    if (!success) {
      log += ' ';
      log += http.errorToString(httpCode);
    }

    if (response.length() > 0) {
      log += ' ';
      log += response.substring(0, 100); // Returned string may be huge, so only log the first part.
    }
    addLogMove(loglevel, log);
  }
}

String send_via_http(const String& logIdentifier,
                     WiFiClient  & client,
                     uint16_t      timeout,
                     const String& user,
                     const String& pass,
                     const String& host,
                     uint16_t      port,
                     const String& uri,
                     const String& HttpMethod,
                     const String& header,
                     const String& postStr,
                     int         & httpCode,
                     bool          must_check_reply) {
  HTTPClient http;
  http.setAuthorization(user.c_str(), pass.c_str());
  http.setTimeout(timeout);
  http.setUserAgent(get_user_agent_string());

  #ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS
  // See: https://github.com/espressif/arduino-esp32/pull/6676
  client.setTimeout((timeout + 500) / 1000); // in seconds!!!!
  #else
  client.setTimeout(timeout); // in msec as it should be!  
  #endif

  // Add request header as fall back.
  // When adding another "accept" header, it may be interpreted as:
  // "if you have XXX, send it; or failing that, just give me what you've got."
  http.addHeader(F("Accept"), F("*/*;q=0.1"));

  delay(0);
#if defined(CORE_POST_2_6_0) || defined(ESP32)
  http.begin(client, host, port, uri, false); // HTTP
#else // if defined(CORE_POST_2_6_0) || defined(ESP32)
  http.begin(client, host, port, uri);
#endif // if defined(CORE_POST_2_6_0) || defined(ESP32)

  const char *keys[] = { "WWW-Authenticate" };
  http.collectHeaders(keys, 1);

  {
    int headerpos = 0;
    String name, value;

    while (splitHeaders(headerpos, header, name, value)) {
      http.addHeader(name, value);
    }
  }

  // start connection and send HTTP header (and body)
  if (HttpMethod.equals(F("HEAD")) || HttpMethod.equals(F("GET"))) {
    httpCode = http.sendRequest(HttpMethod.c_str());
  } else {
    httpCode = http.sendRequest(HttpMethod.c_str(), postStr);
  }

  String response;

  // httpCode will be negative on error
  if (httpCode > 0) {
    const String authReq = http.header(String(F("WWW-Authenticate")).c_str());

    if ((httpCode == 401) && (authReq.indexOf(F("Digest")) != -1)) {
      // Use Digest authorization
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLogMove(LOG_LEVEL_INFO, String(F("HTTP : Start Digest Authorization for ")) + host);
      }

      http.setAuthorization(""); // Clear Basic authorization
      const String authorization = getDigestAuth(authReq, user, pass, "GET", uri, 1);

      http.end();
#if defined(CORE_POST_2_6_0) || defined(ESP32)
      http.begin(client, host, port, uri, false); // HTTP
#else // if defined(CORE_POST_2_6_0) || defined(ESP32)
      http.begin(client, host, port, uri);
#endif // if defined(CORE_POST_2_6_0) || defined(ESP32)

      http.addHeader(F("Authorization"), authorization);

      // start connection and send HTTP header (and body)
      if (HttpMethod.equals(F("HEAD")) || HttpMethod.equals(F("GET"))) {
        httpCode = http.sendRequest(HttpMethod.c_str());
      } else {
        httpCode = http.sendRequest(HttpMethod.c_str(), postStr);
      }
    }

    if (httpCode > 0 && must_check_reply) {
      response = http.getString();
    }
  }
  log_http_result(http, logIdentifier, HttpMethod, httpCode, response);
  http.end();
  if (Settings.UseRules) {
    // Generate event with the HTTP return code
    // e.g. http#hostname=401
    String event = F("http#");
    event += host;
    event += '=';
    event += httpCode;
    eventQueue.addMove(std::move(event));
  }
  return response;
}

String getControllerUser(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings)
{
  if (!validControllerIndex(controller_idx)) { return EMPTY_STRING; }

  if (ControllerSettings.useExtendedCredentials()) {
    return ExtendedControllerCredentials.getControllerUser(controller_idx);
  }
  return SecuritySettings.ControllerUser[controller_idx];
}

String getControllerPass(controllerIndex_t controller_idx, const ControllerSettingsStruct& ControllerSettings)
{
  if (!validControllerIndex(controller_idx)) { return EMPTY_STRING; }

  if (ControllerSettings.useExtendedCredentials()) {
    return ExtendedControllerCredentials.getControllerPass(controller_idx);
  }
  return SecuritySettings.ControllerPassword[controller_idx];
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
  return !getControllerUser(controller_idx, ControllerSettings).isEmpty() &&
         !getControllerPass(controller_idx, ControllerSettings).isEmpty();
}
