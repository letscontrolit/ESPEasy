#ifndef ESPEASY_FWD_DECL_H
#define ESPEASY_FWD_DECL_H


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

struct SettingsStruct;
struct SecurityStruct;
struct CRCStruct;

// Forward declaration to give access to global member variables
SettingsStruct& getSettings();
SecurityStruct& getSecuritySettings();
CRCStruct     & getCRCValues();
unsigned long & getConnectionFailures();
byte          & getHighestActiveLogLevel();
int             getPluginId_from_TaskIndex(byte taskIndex);
float         & getUserVar(unsigned int varIndex);


struct ControllerSettingsStruct;
String   getUnknownString();
void     scheduleNextDelayQueue(unsigned long id,
                                unsigned long nextTime);
String   LoadControllerSettings(int                       ControllerIndex,
                                ControllerSettingsStruct& controller_settings);
String   get_formatted_Controller_number(int controller_index);
void     statusLED(boolean traffic);
void     backgroundtasks();
uint32_t getCurrentFreeStack();
uint32_t getFreeStackWatermark();
bool     canYield();


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


bool   WiFiConnected(uint32_t timeout_ms);
bool   WiFiConnected();
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

#ifdef USES_MQTT

// void runPeriodicalMQTT();
// void updateMQTTclient_connected();
// int firstEnabledMQTTController();
// String getMQTT_state();
void callback(char        *c_topic,
              byte        *b_payload,
              unsigned int length);
void MQTTDisconnect();
bool MQTTConnect(int controller_idx);
bool MQTTCheck(int controller_idx);
void schedule_all_tasks_using_MQTT_controller();
#endif // ifdef USES_MQTT


#endif // ESPEASY_FWD_DECL_H
