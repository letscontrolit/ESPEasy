#ifndef COMMAND_COMMON_H
#define COMMAND_COMMON_H

#include <ctype.h>
#include <Arduino.h>

bool IsNumeric(char * source)
{
  bool result = false;
  if(source)
  {
    int len = strlen(source);
    if(len != 0)  
    {
      int i;
      for(i=0;i<len && isdigit(source[i]);i++);
      result = i == len;
    }
  }
  return result;
}

bool safeReadStringUntil(Stream &input, String &str, char terminator, unsigned int maxSize=1024, unsigned int timeout=1000)
{
    int c;
    const unsigned long timer = millis() + timeout;
    str="";

    do {
        //read character
        c = input.read();
        if(c >= 0) {

            //found terminator, we're ok
            if (c==terminator)
            {
                return(true);
            }
            //found character, add to string
            else
            {
                str+=char(c);
                //string at max size?
                if (str.length()>=maxSize)
                {
                    addLog(LOG_LEVEL_ERROR, F("Not enough bufferspace to read all input data!"));
                    return(false);
                }
            }
        }
        yield();
    } while(!timeOutReached(timer));

    addLog(LOG_LEVEL_ERROR, F("Timeout while reading input data!"));
    return(false);

}

/*
bool Command_GetORSetIP(const __FlashStringHelper *targretDescription,
  const char *Line,
  byte* IP)
{
  return Command_GetORSetIP(targretDescription,Line, IP,1);
}
*/

bool Command_GetORSetIP(const __FlashStringHelper *targretDescription,
  const char *Line,
  byte* IP,
  IPAddress dhcpIP,
  int arg)
{
   bool success = true;
   char TmpStr1[INPUT_COMMAND_SIZE];
   if (GetArgv(Line, TmpStr1, arg+1)) {
    if (!str2ip(TmpStr1,IP))
        Serial.println(F("Invalid parametr."));
   }
   else
   {
     Serial.println();
     Serial.print(targretDescription);
     if(useStaticIP())
     {
       Serial.println(formatIP(IP));
     }
     else
     {
       Serial.print(formatIP(dhcpIP));
       Serial.println(F("(DHCP)"));
     }
   }
   return success;
}

/*
bool Command_GetORSetString(const __FlashStringHelper *targetDescription,
  const char *Line,
  char * target,
  size_t len)
  {
    return Command_GetORSetString(targetDescription,
      Line,
      target,
      len,
      1);
  }
*/

bool Command_GetORSetString(const __FlashStringHelper *targetDescription,
  const char *Line,
  char * target,
  size_t len,
  int arg
  )
  {
   bool success = true;
   char TmpStr1[INPUT_COMMAND_SIZE];
   if (GetArgv(Line, TmpStr1, arg+1)) {
     if(strlen(TmpStr1) > len)
     {
       Serial.println();
       Serial.print(targetDescription);
       Serial.print(F(" is too large. max size is "));
       Serial.println(len);
     }
     else
     {
       strcpy(target,TmpStr1);
     }
   }
   else
   {
     Serial.println();
     Serial.print(targetDescription);
     Serial.println(target);
   }
   return success;
  }

/*
 bool Command_GetORSetBool(const __FlashStringHelper *targretDescription,
  const char *Line,
  bool *value)
  {
    return Command_GetORSetBool(targretDescription,
      Line,
      value,
      1);
  }
*/

  bool Command_GetORSetBool(const __FlashStringHelper *targretDescription,
  const char *Line,
  bool *value,
  int arg)
  {
    bool success = true;
    char TmpStr1[INPUT_COMMAND_SIZE];
    if (GetArgv(Line, TmpStr1, arg + 1)) {
      if(IsNumeric(TmpStr1))
      {
        *value = atoi(TmpStr1) > 0;
      }
      else if(strcmp_P(PSTR("on")  ,TmpStr1) == 0) *value = true;
      else if(strcmp_P(PSTR("true"),TmpStr1) == 0) *value = true;
      else if(strcmp_P(PSTR("off"),TmpStr1) == 0) *value = false;
      else if(strcmp_P(PSTR("false"),TmpStr1) == 0) *value = false;
    }
    else
    {
      Serial.println();
      Serial.print(targretDescription);
      Serial.println(*value ? "true" : "false");
    }
   return success;
  }

#endif // COMMAND_COMMON_H