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

#include "I2CTypes.h"
#include "I2Cdev.h"

#include <FS.h>

#include <WiFiUdp.h>


// Forward declaration to give access to global member variables
float         & getUserVar(unsigned int varIndex);


struct ControllerSettingsStruct;
String   getUnknownString();
void     scheduleNextDelayQueue(unsigned long id,
                                unsigned long nextTime);
String   LoadControllerSettings(controllerIndex_t ControllerIndex,
                                ControllerSettingsStruct& controller_settings);
void     statusLED(bool traffic);
void     backgroundtasks();
uint32_t getCurrentFreeStack();
uint32_t getFreeStackWatermark();
bool     canYield();

void     serialHelper_getGpioNames(struct EventStruct *event,
                                   bool                rxOptional = false,
                                   bool                txOptional = false);

fs::File tryOpenFile(const String& fname,
                     const String& mode);

bool     resolveHostByName(const char *aHostname,
                           IPAddress & aResult);
bool     connectClient(WiFiClient& client,
                       const char *hostname,
                       uint16_t    port);
bool     connectClient(WiFiClient& client,
                       IPAddress   ip,
                       uint16_t    port);


String getWifiModeString(WiFiMode_t wifimode);
bool   NetworkConnected(uint32_t timeout_ms);
bool   NetworkConnected();
bool   useStaticIP();
bool   hostReachable(const IPAddress& ip);
bool   hostReachable(const String& hostname);
void formatMAC(const uint8_t * mac, char (& strMAC)[20]);
String formatMAC(const uint8_t *mac);
String to_json_object_value(const String& object,
                            const String& value);
void htmlEscape(String& html, char c);
void htmlEscape(String& html);


bool     I2C_read_bytes(uint8_t        i2caddr,
                        I2Cdata_bytes& data);
bool     I2C_write8_reg(uint8_t i2caddr,
                        byte    reg,
                        byte    value);
uint8_t  I2C_read8_reg(uint8_t i2caddr,
                       byte    reg,
                       bool   *is_ok = NULL);
uint16_t I2C_read16_reg(uint8_t i2caddr,
                        byte    reg);
int32_t  I2C_read24_reg(uint8_t i2caddr,
                        byte    reg);
uint16_t I2C_read16_LE_reg(uint8_t i2caddr,
                           byte    reg);
int16_t  I2C_readS16_reg(uint8_t i2caddr,
                         byte    reg);
int16_t  I2C_readS16_LE_reg(uint8_t i2caddr,
                            byte    reg);


bool safe_strncpy(char         *dest,
                  const String& source,
                  size_t        max_size);
bool safe_strncpy(char       *dest,
                  const char *source,
                  size_t      max_size);


void rulesProcessing(String& event);
void setIntervalTimer(unsigned long id);
void schedule_notification_event_timer(byte NotificationProtocolIndex, byte Function, struct EventStruct *event);

#ifdef USES_MQTT

// void runPeriodicalMQTT();
// void updateMQTTclient_connected();
controllerIndex_t firstEnabledMQTT_ControllerIndex();
// String getMQTT_state();
void callback(char        *c_topic,
              byte        *b_payload,
              unsigned int length);
void MQTTDisconnect();
bool MQTTConnect(controllerIndex_t controller_idx);
bool MQTTCheck(controllerIndex_t controller_idx);
void schedule_all_tasks_using_MQTT_controller();
bool MQTT_queueFull(controllerIndex_t controller_idx);
bool MQTTpublish(controllerIndex_t controller_idx, const char *topic, const char *payload, bool retained);
#endif // ifdef USES_MQTT


// Used in src/Commands/*
void serialPrintln(const String& text);
void serialPrintln();
bool GetArgv(const char *string, String& argvString, unsigned int argc);
bool HasArgv(const char *string, unsigned int argc);
boolean str2ip(const String& string, byte *IP);
String formatIP(const IPAddress& ip);
String toString(float value, byte decimals);
String boolToString(bool value);
bool isInt(const String& tBuf);
unsigned long hexToUL(const String& input_c);
unsigned long hexToUL(const String& input_c, size_t nrHexDecimals);
unsigned long hexToUL(const String& input_c, size_t startpos, size_t nrHexDecimals);
String formatToHex(unsigned long value, const String& prefix);
String formatToHex(unsigned long value);
String formatToHex_decimal(unsigned long value);
String getNumerical(const String& tBuf, bool mustBeInteger);
String format_msec_duration(long duration);

float getCPUload();
int getLoopCountPerSec();
void serialPrint(const String& text);
void setLogLevelFor(byte destination, byte logLevel);
uint16_t getPortFromKey(uint32_t key);

void initRTC();
boolean saveToRTC();
void deepSleepStart(int dsdelay);
bool setControllerEnableStatus(controllerIndex_t controllerIndex, bool enabled);
bool setTaskEnableStatus(taskIndex_t taskIndex, bool enabled);
void taskClear(taskIndex_t taskIndex, bool save);
void SensorSendTask(taskIndex_t TaskIndex);
bool remoteConfig(struct EventStruct *event, const String& string);

String getControllerParameterInternalName(protocolIndex_t ProtocolIndex, ControllerSettingsStruct::VarType parameterIdx);
void addControllerParameterForm(const ControllerSettingsStruct& ControllerSettings, controllerIndex_t controllerindex, ControllerSettingsStruct::VarType varType);
void saveControllerParameterForm(ControllerSettingsStruct& ControllerSettings, controllerIndex_t controllerindex, ControllerSettingsStruct::VarType varType);

String SaveToFile(SettingsType::Enum settingsType, int index, byte *memAddress, int datasize);
String SaveToFile(SettingsType::Enum settingsType, int index, byte *memAddress, int datasize, int posInBlock);
String LoadFromFile(SettingsType::Enum settingsType, int index, byte *memAddress, int datasize, int offset_in_block);
String LoadFromFile(SettingsType::Enum settingsType, int index, byte *memAddress, int datasize);
String ClearInFile(SettingsType::Enum settingsType, int index);
String LoadStringArray(SettingsType::Enum settingsType, int index, String strings[], uint16_t nrStrings, uint16_t maxStringLength);
String SaveStringArray(SettingsType::Enum settingsType, int index, const String strings[], uint16_t nrStrings, uint16_t maxStringLength);


void SendStatus(EventValueSource::Enum source, const String& status);

String parseTemplate(String& tmpString);
String parseTemplate(String& tmpString, bool useURLencode);
void parseCommandString(struct EventStruct *event, const String& string);

String parseString(const String& string, byte indexFind);
String parseStringKeepCase(const String& string, byte indexFind);
String parseStringToEnd(const String& string, byte indexFind);
String parseStringToEndKeepCase(const String& string, byte indexFind);
String tolerantParseStringKeepCase(const String& string, byte indexFind);

int parseCommandArgumentInt(const String& string, unsigned int argc);
taskIndex_t parseCommandArgumentTaskIndex(const String& string, unsigned int argc);

String describeAllowedIPrange();
void clearAccessBlock();
String rulesProcessingFile(const String& fileName, String& event);
int Calculate(const char *input, float* result);
bool SourceNeedsStatusUpdate(EventValueSource::Enum eventSource);
void SendStatus(EventValueSource::Enum source, const String& status);
bool ExecuteCommand(taskIndex_t taskIndex, EventValueSource::Enum source, const char *Line, bool tryPlugin, bool tryInternal, bool tryRemoteConfig);

void WifiScan(bool async, bool quick = false);
void WifiScan();
void WifiDisconnect();
void setAP(bool enable);
void setSTA(bool enable);

// Used for Networking with Wifi or Ethernet
#include "ESPEasyEthWifi.h"
#include "ESPEasyNetwork.h"
void WiFiConnectRelaxed();
bool WiFiConnected();

#include "src/Globals/ESPEasyWiFiEvent.h"

void setWifiMode(WiFiMode_t wifimode);


String SaveSettings(void);
String LoadSettings();
unsigned long FreeMem(void);
void ResetFactory();
void reboot();
void SendUDPCommand(byte destUnit, const char *data, byte dataLength);
bool hasIPaddr();

#include <FS.h>
void printDirectory(File dir, int numTabs);

void delayBackground(unsigned long dsdelay);

//implemented in Scheduler.ino
void setIntervalTimerOverride(unsigned long id, unsigned long msecFromNow);
void sendGratuitousARP_now();


byte PluginCall(byte Function, struct EventStruct *event, String& str);
bool beginWiFiUDP_randomPort(WiFiUDP& udp);
String toString(float value, byte decimals);

#endif // ESPEASY_FWD_DECL_H
