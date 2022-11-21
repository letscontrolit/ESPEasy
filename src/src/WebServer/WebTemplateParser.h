#ifndef WEBSERVER_WEBTEMPLATEPARSER_H
#define WEBSERVER_WEBTEMPLATEPARSER_H

#include "../../ESPEasy_common.h"

#include <Arduino.h>


#define _HEAD false
#define _TAIL true

#define TASKS_PER_PAGE TASKS_MAX


#define MENU_INDEX_MAIN          0
#define MENU_INDEX_CONFIG        1
#define MENU_INDEX_CONTROLLERS   2
#define MENU_INDEX_HARDWARE      3
#define MENU_INDEX_DEVICES       4
#define MENU_INDEX_RULES         5
#define MENU_INDEX_NOTIFICATIONS 6
#define MENU_INDEX_TOOLS         7
#define MENU_INDEX_SETUP         254
#define MENU_INDEX_CUSTOM_PAGE   255
extern uint8_t navMenuIndex;


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
