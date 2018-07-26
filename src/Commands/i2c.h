#ifndef COMMAND_I2C_H
#define COMMAND_I2C_H


bool Command_i2c_Scanner(struct EventStruct *event, const char* Line)
{
  bool success = true;
  byte error, address;
  for (address = 1; address <= 127; address++ )
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      Serial.print(F("I2C  : Found 0x"));
      Serial.println(String(address, HEX));
    }
    else if (error == 4)
    {
      Serial.print(F("I2C  : Error at 0x"));
      Serial.println(String(address, HEX));
    }
  }
  return success;
}

#endif // COMMAND_I2C_H