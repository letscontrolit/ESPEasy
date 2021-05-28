#ifndef ESPEASYCORE_ESPEASYRULES_H
#define ESPEASYCORE_ESPEASYRULES_H

#define RULE_FILE_SEPARAROR '/'
#define RULE_MAX_FILENAME_LENGTH 24

#include "../../ESPEasy_common.h"

#include "../CustomBuild/ESPEasyLimits.h"


extern boolean activeRuleSets[RULESETS_MAX];


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
 \*********************************************************************************************/
String rulesProcessingFile(const String& fileName,
                           const String& event);


/********************************************************************************************\
   Strip comment from the line.
   Return true when comment was stripped.
 \*********************************************************************************************/
bool rules_strip_trailing_comments(String& line);

/********************************************************************************************\
   Test for common mistake
   Return true if mistake was found (and corrected)
 \*********************************************************************************************/
bool rules_replace_common_mistakes(const String& from,
                                   const String& to,
                                   String      & line);

/********************************************************************************************\
   Check for common mistakes
   Return true if nothing strange found
 \*********************************************************************************************/
bool check_rules_line_user_errors(String& line);


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


void replace_EventValueN_Argv(String      & line,
                              const String& argString,
                              unsigned int  argc);

void substitute_eventvalue(String      & line,
                           const String& event);

void parseCompleteNonCommentLine(String& line,
                                 const String& event,
                                 String& action,
                                 bool  & match,
                                 bool  & codeBlock,
                                 bool  & isCommand,
                                 bool    condition[],
                                 bool    ifBranche[],
                                 byte  & ifBlock,
                                 byte  & fakeIfBlock);

void processMatchedRule(String& action,
                        const String& event,
                        bool  & match,
                        bool  & codeBlock,
                        bool  & isCommand,
                        bool    condition[],
                        bool    ifBranche[],
                        byte  & ifBlock,
                        byte  & fakeIfBlock);

/********************************************************************************************\
   Check if an event matches to a given rule
 \*********************************************************************************************/
bool ruleMatch(const String& event,
               const String& rule);

/********************************************************************************************\
   Check expression
 \*********************************************************************************************/
bool conditionMatchExtended(String& check);

// Find the compare condition.
// @param posStart = first position of the compare condition in the string
// @param posEnd   = first position rest of the string, right after the compare condition.
bool findCompareCondition(const String& check,
                          char        & compare,
                          int         & posStart,
                          int         & posEnd);

bool compareIntValues(char       compare,
                      const int& Value1,
                      const int& Value2);
bool compareDoubleValues(char          compare,
                         const double& Value1,
                         const double& Value2);

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
