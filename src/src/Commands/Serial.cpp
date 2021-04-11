#include "../Commands/Serial.h"

#include "../DataStructs/PinMode.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/Controller.h"
#include "../Commands/Common.h"
#include "../Helpers/StringConverter.h"

String Command_Serial_WriteHex(struct EventStruct *event, const char* Line) {
    String hexdata = tolerantParseStringKeepCase(Line, 2);
    size_t len = hexdata.length();
    if(len == 0) {
        return return_command_failed();
    }

    const char* buf = hexdata.c_str();
    for(unsigned int i = 0; i < len / 2; i++) {
        uint8_t uc = buf[i * 2], dc = buf[i * 2 + 1];
        if(uc >= 48 && uc <= 57) {
            uc -= 48;
        } else if(uc >= 65 && uc <= 70) {
            uc -= 55;
        } else if(uc >= 97 && uc <= 102) {
            uc -= 87;
        } else {
            return return_command_failed();
        }

        if(dc >= 48 && dc <= 57) {
            dc -= 48;
        } else if(dc >= 65 && dc <= 70) {
            dc -= 55;
        } else if(dc >= 97 && dc <= 102) {
            dc -= 87;
        } else {
            return return_command_failed();
        }
        Serial.write(uc << 4 | dc);
    }
    return return_command_success();
}