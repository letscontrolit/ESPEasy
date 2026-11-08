#ifndef ESPEASYCORE_ESPEASYRULES_H
#define ESPEASYCORE_ESPEASYRULES_H

#define RULE_FILE_SEPARAROR '/'
#define RULE_MAX_FILENAME_LENGTH 24

#include "../../ESPEasy_common.h"

#include "../CustomBuild/ESPEasyLimits.h"


#ifdef WEBSERVER_NEW_RULES

String EventToFileName(const String& eventName);

String FileNameToEvent(const String& fileName);
#endif // ifdef WEBSERVER_NEW_RULES

void   checkRuleSets();

/********************************************************************************************\
   Process next event from event queue
 \*********************************************************************************************/
bool   processNextEvent();


/********************************************************************************************\
   Rules processing
 \*********************************************************************************************/
void rulesProcessing(const String& event);

/********************************************************************************************\
   Rules processing
   Return true when event was handled.
 \*********************************************************************************************/
bool rulesProcessingFile(const String& fileName,
                         const String& event,
                         size_t        pos            = 0,
                         bool          startOnMatched = false);


/********************************************************************************************\
   Parse string commands
 \*********************************************************************************************/

void parse_string_commands(String& line);

/********************************************************************************************\
   Generate rule events based on task refresh
 \*********************************************************************************************/
void createRuleEvents(struct EventStruct *event);


#endif // ifndef ESPEASYCORE_ESPEASYRULES_H
