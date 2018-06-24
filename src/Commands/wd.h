#ifndef COMMAND_WD_H
#define COMMAND_WD_H


bool Command_WD_Config(struct EventStruct *event, const char* Line)
{
  bool success = true;
  Wire.beginTransmission(event->Par1);  // address
  Wire.write(event->Par2);              // command
  Wire.write(event->Par3);              // data
  Wire.endTransmission();
  return success;
}

bool Command_WD_Read(struct EventStruct *event, const char* Line)
{
  bool success = true;
  Wire.beginTransmission(event->Par1);  // address
  Wire.write(0x83);                     // command to set pointer
  Wire.write(event->Par2);              // pointer value
  Wire.endTransmission(); 
  if ( Wire.requestFrom((uint8_t)event->Par1, (uint8_t)1) == 1 )
  {
    byte value = Wire.read();
    Serial.println();
    Serial.print(F("I2C Read address "));
    Serial.print(event->Par1,HEX);
    Serial.print(F(" Value "));
    Serial.println(value, HEX);
  }
  return success;
}

#endif // COMMAND_WD_H