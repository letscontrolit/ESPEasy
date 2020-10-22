#ifndef GLOBALS_PLUGIN_OTHER_H
#define GLOBALS_PLUGIN_OTHER_H

#include <Arduino.h>

#include "../CustomBuild/ESPEasyLimits.h"

// This is used to define callbacks to external plugins for various purposes.
// See https://github.com/letscontrolit/ESPEasy/issues/2888


extern void (*parseTemplate_CallBack_ptr)(String& tmpString, bool useURLencode);
extern void (*substitute_eventvalue_CallBack_ptr)(String& line, const String& event);



#endif // GLOBALS_PLUGIN_OTHER_H