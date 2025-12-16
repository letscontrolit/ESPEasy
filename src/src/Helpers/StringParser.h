#ifndef HELPERS_STRINGPARSER_H
#define HELPERS_STRINGPARSER_H

#include "../../ESPEasy_common.h"

#include "../Globals/Plugins.h"

/********************************************************************************************\
   Parse string template
 \*********************************************************************************************/
// Checks str for the given escaped character
bool hasEscapedCharacter(String& str, const char EscapeChar);

// Cleans str from escaped characters
// So far \\% \\[ \\] \\{ \\} \\( and \\) are used (all with single backslash!)
void   stripEscapeCharacters(String& str);

#if FEATURE_STRING_VARIABLES
String parseTemplateAndCalculate(String& tmpString);
uint8_t getDerivedValueCountForTask(taskIndex_t taskIndex);
String getDerivedValueSearchAndPostfix(String  taskName,
                                       String& postfix);
String getDerivedValueNameUomAndVType(String  taskName,
                                      String  valueName,
                                      String& uom,
                                      String& vType);
String getDerivedValueName(String taskName,
                           String valueName);
#endif // if FEATURE_STRING_VARIABLES

String parseTemplate(String& tmpString);

String parseTemplate(String& tmpString,
                     bool    useURLencode);

String parseTemplate_padded(String& tmpString,
                            uint8_t    minimal_lineSize);

String parseTemplate_padded(String& tmpString,
                            uint8_t    minimal_lineSize,
                            bool    useURLencode);


/********************************************************************************************\
   Transform values
 \*********************************************************************************************/

// Syntax: [task#value#transformation#justification]
// valueFormat="transformation#justification"
void transformValue(
  String      & newString,
  uint8_t       lineSize,
  String        value,
  String      & valueFormat,
  const String& tmpString
  #if FEATURE_STRING_VARIABLES
  , taskIndex_t taskIndex  = INVALID_TASK_INDEX
  , uint8_t     valueIndex = INVALID_TASKVAR_INDEX
  , String      valueName  = EMPTY_STRING
  #endif // if FEATURE_STRING_VARIABLES
);



// Find the first (enabled) task with given name
// Return INVALID_TASK_INDEX when not found, else return taskIndex
// deviceName is deepcopy to only store lower case version in cache.
taskIndex_t findTaskIndexByName(String deviceName, bool allowDisabled = false);

// Find the first device value index of a taskIndex.
// Return VARS_PER_TASK if none found.
uint8_t findDeviceValueIndexByName(const String& valueName,
                                taskIndex_t   taskIndex);

// Find positions of [...#...] in the given string.
// Only update pos values on success.
// Return true when found.
bool findNextValMarkInString(const String& input,
                             int         & startpos,
                             int         & hashpos,
                             int         & endpos);

// Find [deviceName#valueName] or [deviceName#valueName#format]
// DeviceName and valueName will be returned in lower case.
// Format may contain case sensitive formatting syntax.
bool findNextDevValNameInString(const String& input,
                                int         & startpos,
                                int         & endpos,
                                String      & deviceName,
                                String      & valueName,
                                String      & format);


/********************************************************************************************\
   Check to see if a given argument is a valid taskIndex (argc = 0 => command)
 \*********************************************************************************************/
taskIndex_t parseCommandArgumentTaskIndex(const String& string,
                                          unsigned int  argc);


/********************************************************************************************\
   Get int from command argument (argc = 0 => command)
 \*********************************************************************************************/
int parseCommandArgumentInt(const String& string,
                            unsigned int  argc,
                            int errorValue = 0);

/********************************************************************************************\
   Parse a command string to event struct
 \*********************************************************************************************/
void parseCommandString(struct EventStruct *event,
                        const String      & string);


#endif