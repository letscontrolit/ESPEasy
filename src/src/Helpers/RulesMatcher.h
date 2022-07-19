#ifndef HELPERS_RULESMATCHER_H
#define HELPERS_RULESMATCHER_H

#include "../../ESPEasy_common.h"

/********************************************************************************************\
   Check if an event matches to a given rule
 \*********************************************************************************************/
// Both strings are copied, since they may need to be trimmed and call parseTemplate
bool ruleMatch(String event,
               String rule);


bool compareIntValues(char       compare,
                      const int& Value1,
                      const int& Value2);

bool compareDoubleValues(char          compare,
                         const double& Value1,
                         const double& Value2);

bool findCompareCondition(const String& check,
                          char        & compare,
                          int         & posStart,
                          int         & posEnd);


// Split a rules line into 2 parts:
// - event: The part between on ... do
// - action: The optional part after the " do"
bool getEventFromRulesLine(const String& line,
                           String      & event,
                           String      & action);

#endif // ifndef HELPERS_RULESMATCHER_H
