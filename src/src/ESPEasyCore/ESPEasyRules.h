#ifndef ESPEASYCORE_ESPEASYRULES_H
#define ESPEASYCORE_ESPEASYRULES_H

#define RULE_FILE_SEPARAROR '/'
#define RULE_MAX_FILENAME_LENGTH 24

#include "../../ESPEasy_common.h"

#include "../CustomBuild/ESPEasyLimits.h"



String EventToFileName(const String& eventName);

String FileNameToEvent(const String& fileName);

void   checkRuleSets();

/********************************************************************************************\
   Process next event from event queue
 \*********************************************************************************************/
bool   processNextEvent();


/********************************************************************************************\
   Rules processing
 \*********************************************************************************************/
void   rulesProcessing(const String& event);

/********************************************************************************************\
   Rules processing
   Return true when event was handled.
 \*********************************************************************************************/
bool rulesProcessingFile(const String& fileName,
                         const String& event,
                         size_t pos = 0,
                         bool   startOnMatched = false);



/********************************************************************************************\
   Parse string commands
 \*********************************************************************************************/

bool get_next_inner_bracket(const String& line,
                            int         & startIndex,
                            int         & closingIndex,
                            char          closingBracket);

bool get_next_argument(const String& fullCommand,
                       int         & index,
                       String      & argument,
                       char          separator);

bool parse_bitwise_functions(const String& cmd_s_lower,
                             const String& arg1,
                             const String& arg2,
                             const String& arg3,
                             uint32_t    & result);
bool parse_math_functions(const String& cmd_s_lower,
                          const String& arg1,
                          const String& arg2,
                          const String& arg3,
                          double      & result);

void parse_string_commands(String& line);


void substitute_eventvalue(String      & line,
                           const String& event);

void parseCompleteNonCommentLine(String& line,
                                 const String& event,
                                 String& action,
                                 bool  & match,
                                 bool  & codeBlock,
                                 bool  & isCommand,
                                 bool  & isOneLiner,
                                 bool    condition[],
                                 bool    ifBranche[],
                                 uint8_t  & ifBlock,
                                 uint8_t  & fakeIfBlock,
                                 bool   startOnMatched);

void processMatchedRule(String& action,
                        const String& event,
                        bool  & isCommand,
                        bool    condition[],
                        bool    ifBranche[],
                        uint8_t  & ifBlock,
                        uint8_t  & fakeIfBlock);


/********************************************************************************************\
   Check expression
 \*********************************************************************************************/
bool conditionMatchExtended(String& check);


bool conditionMatch(const String& check);

/********************************************************************************************\
   Matching time notations HH:MM:SS and HH:MM:SS and HH
 \*********************************************************************************************/

void logtimeStringToSeconds(const String& tBuf,
                            int           hours,
                            int           minutes,
                            int           seconds,
                            bool          valid);

// convert old and new time string to nr of seconds
// return whether it should be considered a time string.
bool timeStringToSeconds(const String& tBuf,
                         int   & time_seconds,
                         String& timeString);


/********************************************************************************************\
   Generate rule events based on task refresh
 \*********************************************************************************************/
void createRuleEvents(struct EventStruct *event);


#endif // ifndef ESPEASYCORE_ESPEASYRULES_H
