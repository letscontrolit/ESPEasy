#ifndef HELPERS_STRINGPARSER_H
#define HELPERS_STRINGPARSER_H

#include <Arduino.h>

#include "../Globals/Plugins.h"

/********************************************************************************************\
   Parse string template
 \*********************************************************************************************/
String parseTemplate(String& tmpString);

String parseTemplate(String& tmpString,
                     bool    useURLencode);

String parseTemplate_padded(String& tmpString,
                            byte    minimal_lineSize);

String parseTemplate_padded(String& tmpString,
                            byte    minimal_lineSize,
                            bool    useURLencode);


/********************************************************************************************\
   Transform values
 \*********************************************************************************************/

// Syntax: [task#value#transformation#justification]
// valueFormat="transformation#justification"
void transformValue(
  String      & newString,
  byte          lineSize,
  String        value,
  String      & valueFormat,
  const String& tmpString);



// Find the first (enabled) task with given name
// Return INVALID_TASK_INDEX when not found, else return taskIndex
taskIndex_t findTaskIndexByName(const String& deviceName);

// Find the first device value index of a taskIndex.
// Return VARS_PER_TASK if none found.
byte findDeviceValueIndexByName(const String& valueName,
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
                            unsigned int  argc);

/********************************************************************************************\
   Parse a command string to event struct
 \*********************************************************************************************/
void parseCommandString(struct EventStruct *event,
                        const String      & string);


#endif