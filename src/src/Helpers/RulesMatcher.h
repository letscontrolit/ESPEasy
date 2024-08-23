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
                      int64_t Value1,
                      int64_t Value2);

bool compareDoubleValues(char          compare,
                         const ESPEASY_RULES_FLOAT_TYPE& Value1,
                         const ESPEASY_RULES_FLOAT_TYPE& Value2,
                         int nrDecimals = -1);

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
