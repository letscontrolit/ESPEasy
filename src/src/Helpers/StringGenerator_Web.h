#ifndef HELPERS_STRINGGENERATOR_WEB_H
#define HELPERS_STRINGGENERATOR_WEB_H

#include "../../ESPEasy_common.h"

void datalistStart(const __FlashStringHelper *id);
void datalistStart(const String& id);
void datalistAddValue(const String& value);
void datalistFinish();
#endif // ifndef HELPERS_STRINGGENERATOR_WEB_H
