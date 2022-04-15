#ifndef HELPERS_RULESMATCHER_H
#define HELPERS_RULESMATCHER_H

#include "../../ESPEasy_common.h"

/********************************************************************************************\
   Check if an event matches to a given rule
 \*********************************************************************************************/
bool ruleMatch(const String& event,
               const String& rule);


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

#endif // ifndef HELPERS_RULESMATCHER_H
