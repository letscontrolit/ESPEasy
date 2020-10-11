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


void     backgroundtasks();


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

bool   NetworkConnected(uint32_t timeout_ms);
bool   hostReachable(const IPAddress& ip);
bool   hostReachable(const String& hostname);
void   updateUDPport();



void rulesProcessing(String& event);
void parse_string_commands(String &line);

#ifdef USES_MQTT

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

// Used in src/Commands/*
void process_serialWriteBuffer();
void serialPrintln(const String& text);
void serialPrintln();



float getCPUload();
int getLoopCountPerSec();
void serialPrint(const String& text);
void setLogLevelFor(byte destination, byte logLevel);


void SensorSendTask(taskIndex_t TaskIndex);


String getControllerParameterInternalName(protocolIndex_t ProtocolIndex, ControllerSettingsStruct::VarType parameterIdx);
void addControllerParameterForm(const ControllerSettingsStruct& ControllerSettings, controllerIndex_t controllerindex, ControllerSettingsStruct::VarType varType);
void saveControllerParameterForm(ControllerSettingsStruct& ControllerSettings, controllerIndex_t controllerindex, ControllerSettingsStruct::VarType varType);

void set_mDNS();


void sendGratuitousARP();
bool processNextEvent();
void sendSysInfoUDP(byte repeats);
void refreshNodeList();
void SSDP_update();





String describeAllowedIPrange();
void clearAccessBlock();
String rulesProcessingFile(const String& fileName, String& event);
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


void SendUDPCommand(byte destUnit, const char *data, byte dataLength);
bool hasIPaddr();

//#include <FS.h>
//void printDirectory(File dir, int numTabs);


bool beginWiFiUDP_randomPort(WiFiUDP& udp);


void Blynk_Run_c015();



// FIXME TD-er: Convert WebServer_Markup.ino to .h/.cpp
void addFormSubHeader(const String& header);
void addSelector_Head(const String& id);
void addSelector_Item(const String& option, int index, boolean selected, boolean disabled, const String& attr);
int getFormItemInt(const String& key, int defaultValue);
void addSelector_Foot();
void addFormSelector(const String& label, const String& id, int optionCount, const String options[], const int indices[], int selectedIndex);

#endif // ESPEASY_FWD_DECL_H
