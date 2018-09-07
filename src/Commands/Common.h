#ifndef COMMAND_COMMON_H
#define COMMAND_COMMON_H

#include <ctype.h>
#include <Arduino.h>

bool IsNumeric(char * source)
{
	bool result = false;
	if (source) {
		int len = strlen(source);
		if (len != 0) {
			int i;
			for (i = 0; i < len && isdigit(source[i]); i++) ;
			result = i == len;
		}
	}
	return result;
}


String Command_GetORSetIP(struct EventStruct *event,
	      const __FlashStringHelper *targetDescription,
			  const char *Line,
			  byte* IP,
			  IPAddress dhcpIP,
			  int arg)
{
	char TmpStr1[INPUT_COMMAND_SIZE];
	if (GetArgv(Line, TmpStr1, arg + 1)) {
		if (!str2ip(TmpStr1, IP)) {
			return return_result(event, F("Invalid parameter."));
		}
	}else  {
		Serial.println();
		String result = targetDescription;
		if (useStaticIP()) {
			result += formatIP(IP);
		}else  {
			result += formatIP(dhcpIP);
			result += F("(DHCP)");
		}
		return return_result(event, result);
	}
	return return_command_success();
}

String Command_GetORSetString(struct EventStruct *event,
	          const __FlashStringHelper *targetDescription,
			      const char *Line,
			      char * target,
			      size_t len,
			      int arg
			      )
{
	char TmpStr1[INPUT_COMMAND_SIZE];
	if (GetArgv(Line, TmpStr1, arg + 1)) {
		if (strlen(TmpStr1) > len) {
			String result = targetDescription;
			result += F(" is too large. max size is ");
			result += len;
			Serial.println();
			return return_result(event, result);
		}else  {
			strcpy(target, TmpStr1);
		}
	}else  {
		Serial.println();
		String result = targetDescription;
		result += target;
		return return_result(event, result);
	}
	return return_command_success();
}

String Command_GetORSetBool(struct EventStruct *event,
	        const __FlashStringHelper *targetDescription,
			    const char *Line,
			    bool *value,
			    int arg)
{
	char TmpStr1[INPUT_COMMAND_SIZE];
	if (GetArgv(Line, TmpStr1, arg + 1)) {
		if (IsNumeric(TmpStr1)) {
			*value = atoi(TmpStr1) > 0;
		}
    else if (strcmp_P(PSTR("on"), TmpStr1) == 0) *value = true;
		else if (strcmp_P(PSTR("true"), TmpStr1) == 0) *value = true;
		else if (strcmp_P(PSTR("off"), TmpStr1) == 0) *value = false;
		else if (strcmp_P(PSTR("false"), TmpStr1) == 0) *value = false;
	}else  {
		String result = targetDescription;
		result += toString(*value);
		return return_result(event, result);
	}
	return return_command_success();
}

#endif // COMMAND_COMMON_H
