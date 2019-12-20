#ifndef ESPEASY_FWD_DECL_H
#define ESPEASY_FWD_DECL_H

#include "ESPEasy_common.h"
#include "src/DataStructs/ESPEasy_EventStruct.h"

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


// Forward declaration to give access to global member variables
float         & getUserVar(unsigned int varIndex);


struct ControllerSettingsStruct;
String   getUnknownString(void);
void     scheduleNextDelayQueue(unsigned long id,
                                unsigned long nextTime);
String   LoadControllerSettings(int                       ControllerIndex,
                                ControllerSettingsStruct& controller_settings);
String   get_formatted_Controller_number(int controller_index);
void     statusLED(bool traffic);
void     backgroundtasks(void);
uint32_t getCurrentFreeStack(void);
uint32_t getFreeStackWatermark(void);
bool     canYield(void);


boolean  timeOutReached(unsigned long timer);
long     timePassedSince(unsigned long timestamp);
long     usecPassedSince(unsigned long timestamp);

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
bool   WiFiConnected(uint32_t timeout_ms);
bool   WiFiConnected(void);
bool   useStaticIP(void);
bool   hostReachable(const IPAddress& ip);
bool   hostReachable(const String& hostname);
void formatMAC(const uint8_t * mac, char (& strMAC)[20]);
String to_json_object_value(const String& object,
                            const String& value);


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
byte getProtocolIndex(byte Number);
byte getNotificationProtocolIndex(byte Number);
void schedule_notification_event_timer(byte NotificationProtocolIndex, byte Function, struct EventStruct *event);

#ifdef USES_MQTT

// void runPeriodicalMQTT(void);
// void updateMQTTclient_connected(void);
int firstEnabledMQTTController(void);
// String getMQTT_state(void);
void callback(char        *c_topic,
              byte        *b_payload,
              unsigned int length);
void MQTTDisconnect(void);
bool MQTTConnect(int controller_idx);
bool MQTTCheck(int controller_idx);
void schedule_all_tasks_using_MQTT_controller(void);
bool MQTTpublish(int controller_idx, const char *topic, const char *payload, boolean retained);
#endif // ifdef USES_MQTT


// Used in src/Commands/*
void serialPrintln(const String& text);
void serialPrintln(void);
bool GetArgv(const char *string, String& argvString, unsigned int argc);
bool HasArgv(const char *string, unsigned int argc);
boolean str2ip(const String& string, byte *IP);
bool useStaticIP(void);
String formatIP(const IPAddress& ip);
String toString(float value, byte decimals);
String boolToString(bool value);
bool isInt(const String& tBuf);
String formatToHex(unsigned long value, const String& prefix);
String formatToHex(unsigned long value);

float getCPUload(void);
int getLoopCountPerSec(void);
void serialPrint(const String& text);
void setLogLevelFor(byte destination, byte logLevel);
uint16_t getPortFromKey(uint32_t key);

void initRTC(void);
void deepSleepStart(int dsdelay);
void taskClear(taskIndex_t taskIndex, bool save);
void SensorSendTask(taskIndex_t TaskIndex);
bool remoteConfig(struct EventStruct *event, const String& string);

String parseString(const String& string, byte indexFind);
String parseStringKeepCase(const String& string, byte indexFind);
String parseStringToEnd(const String& string, byte indexFind);
String parseStringToEndKeepCase(const String& string, byte indexFind);
String tolerantParseStringKeepCase(const String& string, byte indexFind);

int parseCommandArgumentInt(const String& string, unsigned int argc);

String describeAllowedIPrange(void);
void clearAccessBlock(void);
String rulesProcessingFile(const String& fileName, String& event);
int Calculate(const char *input, float* result);
bool SourceNeedsStatusUpdate(byte eventSource);

void WifiScan(bool async, bool quick = false);
void WifiScan(void);
void WiFiConnectRelaxed(void);
void WifiDisconnect(void);
void evaluateConnectionFailures(void);
void setAP(bool enable);
void setSTA(bool enable);

#include "src/Globals/ESPEasyWiFiEvent.h"

void setWifiMode(WiFiMode_t wifimode);


String SaveSettings(void);
String LoadSettings(void);
unsigned long FreeMem(void);
void ResetFactory(void);
void reboot(void);
void SendUDPCommand(byte destUnit, const char *data, byte dataLength);

#include <FS.h>
void printDirectory(File dir, int numTabs);

void delayBackground(unsigned long dsdelay);

void setIntervalTimerOverride(unsigned long id, unsigned long msecFromNow); //implemented in Scheduler.ino

#endif // ESPEASY_FWD_DECL_H
