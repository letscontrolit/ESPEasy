#ifndef WEBSERVER_WEBSERVER_COMMON_H
#define WEBSERVER_WEBSERVER_COMMON_H


#include <Arduino.h>
#include <FS.h>

#ifdef ESP32
  #include <WiFi.h>
  #include <WebServer.h>
#endif

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
#endif

#include "../../ESPEasy_common.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Services.h"
#include "../Globals/TXBuffer.h"
#include "../Helpers/StringProvider.h"

void sendHeader(const String& name, const String& value, bool first = false);
void sendHeader(const __FlashStringHelper * name, const String& value, bool first = false);
void sendHeader(const __FlashStringHelper * name, const __FlashStringHelper * value, bool first = false);

// Separate wrapper to get web_server.arg()
// 1) To allow to have a __FlashStringHelper call -> reduce build size
// 2) ESP32 does not return a const String &, but a temporary copy, thus we _must_ copy before using it.

#ifdef ESP8266
const String& webArg(const __FlashStringHelper * arg);
const String& webArg(const String& arg);
const String& webArg(int i);
#endif 

#ifdef ESP32
String webArg(const __FlashStringHelper * arg);
String webArg(const String& arg);
String webArg(int i);
#endif 

bool hasArg(const __FlashStringHelper * arg);
bool hasArg(const String& arg);

#endif