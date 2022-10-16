#include "../DataStructs/ControllerSettingsStruct.h"

#include "../../ESPEasy_common.h"

#include "../CustomBuild/ESPEasyLimits.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringConverter.h"


#include <IPAddress.h>
#include <WString.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>


ControllerSettingsStruct::ControllerSettingsStruct()
{
  reset();
}

void ControllerSettingsStruct::reset() {
  UseDNS                     = DEFAULT_SERVER_USEDNS;
  Port                       = DEFAULT_PORT;
  MinimalTimeBetweenMessages = CONTROLLER_DELAY_QUEUE_DELAY_DFLT;
  MaxQueueDepth              = CONTROLLER_DELAY_QUEUE_DEPTH_DFLT;
  MaxRetry                   = CONTROLLER_DELAY_QUEUE_RETRY_DFLT;
  DeleteOldest               = DEFAULT_CONTROLLER_DELETE_OLDEST;
  ClientTimeout              = CONTROLLER_CLIENTTIMEOUT_DFLT;
  MustCheckReply             = DEFAULT_CONTROLLER_MUST_CHECK_REPLY ;
  SampleSetInitiator         = INVALID_TASK_INDEX;
  VariousFlags               = 0;

  for (uint8_t i = 0; i < 4; ++i) {
    IP[i] = 0;
  }
  ZERO_FILL(HostName);
  ZERO_FILL(ClientID);
  ZERO_FILL(Publish);
  ZERO_FILL(Subscribe);
  ZERO_FILL(MQTTLwtTopic);
  ZERO_FILL(LWTMessageConnect);
  ZERO_FILL(LWTMessageDisconnect);
  safe_strncpy(ClientID, F(CONTROLLER_DEFAULT_CLIENTID), sizeof(ClientID));
}

bool ControllerSettingsStruct::isSet() const {
  if (UseDNS) {
    return HostName[0] != 0;
  }
  return ipSet();
}

void ControllerSettingsStruct::validate() {
  if (Port > 65535) { Port = 0; }

  if ((MinimalTimeBetweenMessages < 1) ||  (MinimalTimeBetweenMessages > CONTROLLER_DELAY_QUEUE_DELAY_MAX)) {
    MinimalTimeBetweenMessages = CONTROLLER_DELAY_QUEUE_DELAY_DFLT;
  }

  if (MaxQueueDepth > CONTROLLER_DELAY_QUEUE_DEPTH_MAX) { MaxQueueDepth = CONTROLLER_DELAY_QUEUE_DEPTH_DFLT; }

  if (MaxRetry > CONTROLLER_DELAY_QUEUE_RETRY_MAX) { MaxRetry = CONTROLLER_DELAY_QUEUE_RETRY_MAX; }

  if (MaxQueueDepth == 0) { MaxQueueDepth = CONTROLLER_DELAY_QUEUE_DEPTH_DFLT; }

  if (MaxRetry == 0) { MaxRetry = CONTROLLER_DELAY_QUEUE_RETRY_DFLT; }

  if ((ClientTimeout < 10) || (ClientTimeout > CONTROLLER_CLIENTTIMEOUT_MAX)) {
    ClientTimeout = CONTROLLER_CLIENTTIMEOUT_DFLT;
  }
  ZERO_TERMINATE(HostName);
  ZERO_TERMINATE(Publish);
  ZERO_TERMINATE(Subscribe);
  ZERO_TERMINATE(MQTTLwtTopic);
  ZERO_TERMINATE(LWTMessageConnect);
  ZERO_TERMINATE(LWTMessageDisconnect);

  #if FEATURE_MQTT
    #if FEATURE_MQTT_TLS
    if (TLStype() == TLS_types::NoTLS) {
      if (Port == 8883) {
        Port = 1883;
        addLog(LOG_LEVEL_ERROR, F("Not using TLS, but port set to secure 8883. Use port 1883 instead"));
      }
    } else {
      if (Port == 1883) {
        Port = 8883;
        addLog(LOG_LEVEL_ERROR, F("Using TLS, but port set to insecure port 1883. Use port 8883 instead"));
      }
    }
    #else
    if (Port == 8883) {
      // No TLS support, so when switching builds, make sure it can still work.
      Port = 1883;
      addLog(LOG_LEVEL_ERROR, F("Not using TLS, but port set to secure 8883. Use port 1883 instead"));
    }
    #endif
  #endif

}

IPAddress ControllerSettingsStruct::getIP() const {
  IPAddress host(IP[0], IP[1], IP[2], IP[3]);

  return host;
}

String ControllerSettingsStruct::getHost() const {
  if (UseDNS) {
    return HostName;
  }
  return getIP().toString();
}

void ControllerSettingsStruct::setHostname(const String& controllerhostname) {
  safe_strncpy(HostName, controllerhostname.c_str(), sizeof(HostName));
  updateIPcache();
}

bool ControllerSettingsStruct::checkHostReachable(bool quick) {
  if (!isSet()) {
    // No IP/hostname set
    return false;
  }
  if (!NetworkConnected(10)) {
    return false; // Not connected, so no use in wasting time to connect to a host.
  }
  delay(0);       // Make sure the Watchdog will not trigger a reset.

  if (quick && ipSet()) { return true; }

  if (UseDNS) {
    if (!updateIPcache()) {
      return false;
    }
  }
  return hostReachable(getIP());
}

#if FEATURE_HTTP_CLIENT
bool ControllerSettingsStruct::connectToHost(WiFiClient& client) {
  if (!checkHostReachable(true)) {
    return false; // Host not reachable
  }
  uint8_t retry     = 2;
  bool connected = false;

  while (retry > 0 && !connected) {
    --retry;
    connected = connectClient(client, getIP(), Port, ClientTimeout);

    if (connected) { return true; }

    if (!checkHostReachable(false)) {
      return false;
    }
  }
  return false;
}
#endif // FEATURE_HTTP_CLIENT

bool ControllerSettingsStruct::beginPacket(WiFiUDP& client) {
  if (!checkHostReachable(true)) {
    return false; // Host not reachable
  }
  uint8_t retry     = 2;
  while (retry > 0) {
    --retry;
    FeedSW_watchdog();
    if (client.beginPacket(getIP(), Port) == 1) {
      return true;
    }

    if (!checkHostReachable(false)) {
      return false;
    }
    delay(10);
  }
  return false;
}

String ControllerSettingsStruct::getHostPortString() const {
  String result = getHost();

  result += ':';
  result += Port;
  return result;
}

bool ControllerSettingsStruct::ipSet() const {
  for (uint8_t i = 0; i < 4; ++i) {
    if (IP[i] != 0) { return true; }
  }
  return false;
}

bool ControllerSettingsStruct::updateIPcache() {
  if (!UseDNS) {
    return true;
  }

  if (!NetworkConnected()) { return false; }
  IPAddress tmpIP;

  if (resolveHostByName(HostName, tmpIP, ClientTimeout)) {
    for (uint8_t x = 0; x < 4; x++) {
      IP[x] = tmpIP[x];
    }
    return true;
  }
  return false;
}

#if FEATURE_MQTT
bool ControllerSettingsStruct::mqtt_cleanSession() const
{
  return bitRead(VariousFlags, 1);
}

void ControllerSettingsStruct::mqtt_cleanSession(bool value)
{
  bitWrite(VariousFlags, 1, value);
}

bool ControllerSettingsStruct::mqtt_sendLWT() const
{
  return !bitRead(VariousFlags, 2);
}

void ControllerSettingsStruct::mqtt_sendLWT(bool value)
{
  bitWrite(VariousFlags, 2, !value);
}

bool ControllerSettingsStruct::mqtt_willRetain() const
{
  return !bitRead(VariousFlags, 3);
}

void ControllerSettingsStruct::mqtt_willRetain(bool value)
{
  bitWrite(VariousFlags, 3, !value);
}

bool ControllerSettingsStruct::mqtt_uniqueMQTTclientIdReconnect() const
{
  return bitRead(VariousFlags, 4);
}

void ControllerSettingsStruct::mqtt_uniqueMQTTclientIdReconnect(bool value)
{
  bitWrite(VariousFlags, 4, value);
}

bool ControllerSettingsStruct::mqtt_retainFlag() const
{
  return bitRead(VariousFlags, 5);
}

void ControllerSettingsStruct::mqtt_retainFlag(bool value)
{
  bitWrite(VariousFlags, 5, value);
}
#endif

bool ControllerSettingsStruct::useExtendedCredentials() const
{
  return bitRead(VariousFlags, 6);
}

void ControllerSettingsStruct::useExtendedCredentials(bool value)
{
  bitWrite(VariousFlags, 6, value);
}

bool ControllerSettingsStruct::sendBinary() const
{
  return bitRead(VariousFlags, 7);
}

void ControllerSettingsStruct::sendBinary(bool value)
{
  bitWrite(VariousFlags, 7, value);
}

bool ControllerSettingsStruct::allowExpire() const
{
  return bitRead(VariousFlags, 9);
}

void ControllerSettingsStruct::allowExpire(bool value)
{
  bitWrite(VariousFlags, 9, value);
}

bool ControllerSettingsStruct::deduplicate() const
{
  return bitRead(VariousFlags, 10);
}

void ControllerSettingsStruct::deduplicate(bool value)
{
  bitWrite(VariousFlags, 10, value);
}

bool ControllerSettingsStruct::useLocalSystemTime() const
{
  return bitRead(VariousFlags, 11);
}

void ControllerSettingsStruct::useLocalSystemTime(bool value)
{
  bitWrite(VariousFlags, 11, value);
}

#if FEATURE_MQTT_TLS
TLS_types ControllerSettingsStruct::TLStype() const
{
  // Store it in bits 12, 13, 14, 15
  return static_cast<TLS_types>(get4BitFromUL(VariousFlags, 12));
}

void  ControllerSettingsStruct::TLStype(TLS_types tls_type)
{
  set4BitToUL(VariousFlags, 12, static_cast<uint8_t>(tls_type));
}

String ControllerSettingsStruct::getCertificateFilename() const
{
  return getCertificateFilename(TLStype());
}

String ControllerSettingsStruct::getCertificateFilename(TLS_types tls_type) const
{
  String certFile = HostName;
  if (certFile.isEmpty()) {
    certFile = F("<HostName>");
  }

  switch (tls_type) {
    case TLS_types::NoTLS:
    case TLS_types::TLS_insecure:
      return EMPTY_STRING;
    case TLS_types::TLS_PSK:
      certFile += F(".psk");
      break;
    /*
    case TLS_types::TLS_CA_CLI_CERT:
      certFile += F(".caclicert");
      break;
    */
    case TLS_types::TLS_CA_CERT:
      certFile += F(".cacert");
      break;
    case TLS_types::TLS_FINGERPRINT:
      certFile += F(".fp");
      break;
  }

  // Only use the last 29 bytes of the filename
  if (certFile.length() > 28) {
    certFile = certFile.substring(certFile.length() - 28);
  }
  
  return certFile;
}
#endif