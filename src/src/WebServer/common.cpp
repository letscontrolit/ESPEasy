#include "../WebServer/common.h"

void sendHeader(const String& name, const String& value, bool first)
{
    web_server.sendHeader(name, value, first);
}

void sendHeader(const __FlashStringHelper * name, const String& value, bool first)
{
    web_server.sendHeader(name, value, first);
}

void sendHeader(const __FlashStringHelper * name, const __FlashStringHelper * value, bool first)
{
    web_server.sendHeader(name, value, first);
}


#ifdef ESP8266
const String& webArg(const __FlashStringHelper * arg)
{
  return web_server.arg(String(arg));
}

const String& webArg(const String& arg)
{
  return web_server.arg(arg);
}

const String& webArg(int i)
{
  return web_server.arg(i);
}
#endif

#ifdef ESP32
String webArg(const __FlashStringHelper * arg)
{
  return web_server.arg(String(arg));
}

String webArg(const String& arg)
{
  return web_server.arg(arg);
}

String webArg(int i)
{
  return web_server.arg(i);
}
#endif

bool hasArg(const __FlashStringHelper * arg)
{
  return web_server.hasArg(arg);
}

bool hasArg(const String& arg)
{
  return web_server.hasArg(arg);
}
