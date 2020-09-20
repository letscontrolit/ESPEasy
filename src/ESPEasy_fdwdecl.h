#ifndef ESPEASY_FWD_DECL_H
#define ESPEASY_FWD_DECL_H

#include "ESPEasy_common.h"
#include "src/DataStructs/SettingsType.h"
#include "src/DataStructs/ESPEasy_EventStruct.h"

#include "src/Globals/CPlugins.h"

// FIXME TD-er: This header file should only be included from .ino or .cpp files
// This is only needed until the classes that need these can include the appropriate .h files to have these forward declared.




#if defined(ESP8266)

//  #include <lwip/init.h>
  # include <ESP8266WiFi.h>
  # include <ESP8266WebServer.h>
#endif // if defined(ESP8266)
#if defined(ESP32)
  # include <WiFi.h>
  # include <WebServer.h>
#endif // if defined(ESP32)

#include <FS.h>

#include <WiFiUdp.h>


void     statusLED(bool traffic);
void     backgroundtasks();
uint32_t getCurrentFreeStack();
uint32_t getFreeStackWatermark();


/*
struct ControllerSettingsStruct;
String   getUnknownString();
bool     canYield();
*/


void     serialHelper_getGpioNames(struct EventStruct *event,
                                   bool                rxOptional = false,
                                   bool                txOptional = false);
/*
fs::File tryOpenFile(const String& fname,
                     const String& mode);
*/


bool     resolveHostByName(const char *aHostname,
                           IPAddress & aResult);

bool     connectClient(WiFiClient& client,
                       const char *hostname,
                       uint16_t    port);

bool     connectClient(WiFiClient& client,
                       IPAddress   ip,
                       uint16_t    port);

bool   useStaticIP();
String getWifiModeString(WiFiMode_t wifimode);
bool   NetworkConnected(uint32_t timeout_ms);
bool   hostReachable(const IPAddress& ip);
bool   hostReachable(const String& hostname);
void   updateUDPport();


bool safe_strncpy(char         *dest,
                  const String& source,
                  size_t        max_size);
bool safe_strncpy(char       *dest,
                  const char *source,
                  size_t      max_size);


void rulesProcessing(String& event);

#ifdef USES_MQTT

// void runPeriodicalMQTT();
// void updateMQTTclient_connected();
//controllerIndex_t firstEnabledMQTT_ControllerIndex();
String getMQTT_state();
void callback(char        *c_topic,
              byte        *b_payload,
              unsigned int length);
//void MQTTDisconnect();
//bool MQTTConnect(controllerIndex_t controller_idx);
bool MQTTCheck(controllerIndex_t controller_idx);
//bool MQTT_queueFull(controllerIndex_t controller_idx);
bool MQTTpublish(controllerIndex_t controller_idx, const char *topic, const char *payload, bool retained);
#endif // ifdef USES_MQTT

void flushAndDisconnectAllClients();
bool saveUserVarToRTC();

// Used in src/Commands/*
void process_serialWriteBuffer();
void serialPrintln(const String& text);
void serialPrintln();



float getCPUload();
int getLoopCountPerSec();
void serialPrint(const String& text);
void setLogLevelFor(byte destination, byte logLevel);
uint16_t getPortFromKey(uint32_t key);

void initRTC();
boolean saveToRTC();

bool setControllerEnableStatus(controllerIndex_t controllerIndex, bool enabled);
bool setTaskEnableStatus(taskIndex_t taskIndex, bool enabled);
void taskClear(taskIndex_t taskIndex, bool save);
void SensorSendTask(taskIndex_t TaskIndex);
bool remoteConfig(struct EventStruct *event, const String& string);

String getTaskDeviceName(taskIndex_t TaskIndex);

String getControllerParameterInternalName(protocolIndex_t ProtocolIndex, ControllerSettingsStruct::VarType parameterIdx);
void addControllerParameterForm(const ControllerSettingsStruct& ControllerSettings, controllerIndex_t controllerindex, ControllerSettingsStruct::VarType varType);
void saveControllerParameterForm(ControllerSettingsStruct& ControllerSettings, controllerIndex_t controllerindex, ControllerSettingsStruct::VarType varType);

void set_mDNS();
bool allocatedOnStack(const void* address);

uint32_t createKey(uint16_t pluginNumber, uint16_t portNumber);

void sendGratuitousARP();
bool processNextEvent();
void delayedReboot(int rebootDelay);
void sendSysInfoUDP(byte repeats);
void refreshNodeList();
void SSDP_update();


String parseTemplate(String& tmpString);
String parseTemplate(String& tmpString, bool useURLencode);
void parseCommandString(struct EventStruct *event, const String& string);

String parseTemplate_padded(String& tmpString, byte minimal_lineSize);

/*
String parseString(const String& string, byte indexFind);
String parseStringKeepCase(const String& string, byte indexFind);
String parseStringToEnd(const String& string, byte indexFind);
String parseStringToEndKeepCase(const String& string, byte indexFind);
String tolerantParseStringKeepCase(const String& string, byte indexFind);
*/

int parseCommandArgumentInt(const String& string, unsigned int argc);
taskIndex_t parseCommandArgumentTaskIndex(const String& string, unsigned int argc);

String describeAllowedIPrange();
void clearAccessBlock();
String rulesProcessingFile(const String& fileName, String& event);
int Calculate(const char *input, float* result);
bool SourceNeedsStatusUpdate(EventValueSource::Enum eventSource);
void SendStatus(EventValueSource::Enum source, const String& status);
//bool ExecuteCommand(taskIndex_t taskIndex, EventValueSource::Enum source, const char *Line, bool tryPlugin, bool tryInternal, bool tryRemoteConfig);

//void WifiScan(bool async, bool quick = false);
//void WifiScan();
//void WifiDisconnect();
//void setAP(bool enable);
//void setSTA(bool enable);

// Used for Networking with Wifi or Ethernet
//#include "ESPEasyEthWiFi.h"
#include "ESPEasyNetwork.h"
//void WiFiConnectRelaxed();
//bool WiFiConnected();

#include "src/Globals/ESPEasyWiFiEvent.h"

//void setWifiMode(WiFiMode_t wifimode);

unsigned long FreeMem(void);
void ResetFactory();
void reboot();
void SendUDPCommand(byte destUnit, const char *data, byte dataLength);
bool hasIPaddr();

//#include <FS.h>
//void printDirectory(File dir, int numTabs);

void delayBackground(unsigned long dsdelay);



byte PluginCall(byte Function, struct EventStruct *event, String& str);
bool beginWiFiUDP_randomPort(WiFiUDP& udp);

uint8_t get8BitFromUL(uint32_t number, byte bitnr);

void Blynk_Run_c015();

#endif // ESPEASY_FWD_DECL_H
