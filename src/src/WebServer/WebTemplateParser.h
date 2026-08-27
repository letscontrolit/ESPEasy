#ifndef WEBSERVER_WEBTEMPLATEPARSER_H
#define WEBSERVER_WEBTEMPLATEPARSER_H

#include "../../ESPEasy_common.h"

#define _HEAD false
#define _TAIL true

#define TASKS_PER_PAGE TASKS_MAX


#define MENU_INDEX_MAIN          0
#define MENU_INDEX_CONFIG        1
#define MENU_INDEX_HARDWARE      2
#define MENU_INDEX_INTERFACES    3
#define MENU_INDEX_INTERFACES_I2C     30
#define MENU_INDEX_INTERFACES_SPI     31
#define MENU_INDEX_INTERFACES_MODBUS  32    // For now Modbus-RTU, maybe later also Modbus-TCP
#define MENU_INDEX_INTERFACES_CAN     33
#define MENU_INDEX_INTERFACES_WRMBUS  34    // Wired M-Bus
#define MENU_INDEX_INTERFACES_WIMBUS  35    // Wireless M-Bus
#define MENU_INDEX_NETWORK       4
#define MENU_INDEX_CONTROLLERS   5
#define MENU_INDEX_BUSES         6
#define MENU_INDEX_DEVICES       7
#define MENU_INDEX_RULES         8
#define MENU_INDEX_NOTIFICATIONS 9
#define MENU_INDEX_TOOLS         10
#define MENU_INDEX_IGNORE        253
#define MENU_INDEX_SETUP         254
#define MENU_INDEX_CUSTOM_PAGE   255

#define MENU_MAX_INDEX_SHOWN     MENU_INDEX_TOOLS
#define MENU_MIN_INTERFACES_SHOWN MENU_INDEX_INTERFACES_I2C
#define MENU_MAX_INTERFACES_SHOWN MENU_INDEX_INTERFACES_WIMBUS

extern uint8_t navMenuIndex;

bool isGpMenuSecondLevel(uint8_t index);
const __FlashStringHelper* getGpMenuIcon(uint8_t index);
const __FlashStringHelper* getGpMenuLabel(uint8_t index);

class WebTemplateParser {
public:

  WebTemplateParser(bool tail, bool rebooting) : Tail(tail), Rebooting(rebooting) {}

  bool process(const char c);

  bool process(const __FlashStringHelper * pstr);
  bool process(PGM_P str);
  bool process(const String& str);

  bool isTail() const { return Tail; }

private:

  void processVarName();

  void getErrorNotifications();

  void getWebPageTemplateVar(const String& varName);

  String varName;
  char prev = '\0';

  const bool Tail      = false;
  const bool Rebooting = false;
  bool contentVarFound = false;
  bool parsingVarName = false;
};


#endif // ifndef WEBSERVER_WEBTEMPLATEPARSER_H
