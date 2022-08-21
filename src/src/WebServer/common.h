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

#endif