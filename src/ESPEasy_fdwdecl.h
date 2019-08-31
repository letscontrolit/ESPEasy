#ifndef ESPEASY_FWD_DECL_H
#define ESPEASY_FWD_DECL_H

#if defined(ESP8266)
//  #include <lwip/init.h>
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
#endif
#if defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
#endif

#include "I2CTypes.h"
#include "I2Cdev.h"



// Forward declaration
struct ControllerSettingsStruct;
static String getUnknownString();
void scheduleNextDelayQueue(unsigned long id, unsigned long nextTime);
String LoadControllerSettings(int ControllerIndex, ControllerSettingsStruct& controller_settings);
String get_formatted_Controller_number(int controller_index);
bool loglevelActiveFor(byte logLevel);
void addToLog(byte loglevel, const String& string);
void addToLog(byte logLevel, const __FlashStringHelper* flashString);
void statusLED(boolean traffic);
void backgroundtasks();
uint32_t getCurrentFreeStack();
uint32_t getFreeStackWatermark();
bool canYield();

bool getBitFromUL(uint32_t number, byte bitnr);
void setBitToUL(uint32_t& number, byte bitnr, bool value);

void serialHelper_getGpioNames(struct EventStruct *event, bool rxOptional=false, bool txOptional=false);

fs::File tryOpenFile(const String& fname, const String& mode);

bool resolveHostByName(const char* aHostname, IPAddress& aResult);
bool connectClient(WiFiClient& client, const char* hostname, uint16_t port);
bool connectClient(WiFiClient& client, IPAddress ip, uint16_t port);




bool WiFiConnected(uint32_t timeout_ms);
bool WiFiConnected();
bool hostReachable(const IPAddress& ip);
bool hostReachable(const String& hostname);
void formatMAC(const uint8_t* mac, char (&strMAC)[20]);
String to_json_object_value(const String& object, const String& value);


bool I2C_read_bytes(uint8_t i2caddr, I2Cdata_bytes& data);
bool I2C_write8_reg(uint8_t i2caddr, byte reg, byte value);
uint8_t I2C_read8_reg(uint8_t i2caddr, byte reg, bool * is_ok = NULL);
uint16_t I2C_read16_reg(uint8_t i2caddr, byte reg);
int32_t I2C_read24_reg(uint8_t i2caddr, byte reg);
uint16_t I2C_read16_LE_reg(uint8_t i2caddr, byte reg);
int16_t I2C_readS16_reg(uint8_t i2caddr, byte reg);
int16_t I2C_readS16_LE_reg(uint8_t i2caddr, byte reg);
I2Cdev i2cdev;


bool safe_strncpy(char* dest, const String& source, size_t max_size);
bool safe_strncpy(char* dest, const char* source, size_t max_size);


#endif // ESPEASY_FWD_DECL_H