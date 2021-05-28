#include "../Commands/wd.h"

#ifndef LIMIT_BUILD_SIZE

#include "../Commands/Common.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../ESPEasyCore/Serial.h"

#include "../Helpers/StringConverter.h"


const __FlashStringHelper * Command_WD_Config(EventStruct *event, const char* Line)
{
  Wire.beginTransmission(event->Par1);  // address
  Wire.write(event->Par2);              // command
  Wire.write(event->Par3);              // data
  Wire.endTransmission();
  return return_command_success();
}

String Command_WD_Read(EventStruct *event, const char* Line)
{
  Wire.beginTransmission(event->Par1);  // address
  Wire.write(0x83);                     // command to set pointer
  Wire.write(event->Par2);              // pointer value
  Wire.endTransmission();
  if ( Wire.requestFrom(static_cast<uint8_t>(event->Par1), static_cast<uint8_t>(1)) == 1 )
  {
    byte value = Wire.read();
    serialPrintln();
    String result = F("I2C Read address ");
    result += formatToHex(event->Par1);
    result += F(" Value ");
    result += formatToHex(value);
    return return_result(event, result);
  }
  return return_command_success();
}

#endif