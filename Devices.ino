// These are output devices that do not need a device plugin (yet).
// May change in the future if we decide that all IO should be done through plugins.

/*********************************************************************************************\
 * Extender (Arduino Mini Pro connected through I2C)
 * The Mini Pro needs a small sketch to support this
\*********************************************************************************************/
int extender(byte Cmd, byte Port, int Value)
{ 
  uint8_t address = 0x7f;
  Wire.beginTransmission(address);
  Wire.write(Cmd);
  Wire.write(Port);
  Wire.write(Value & 0xff);
  Wire.write((Value >> 8));
  Wire.endTransmission();
  if (Cmd == 2 || Cmd == 4)
  {
    delay(1);  // remote unit needs some time to do the adc stuff
    Wire.requestFrom(address, (uint8_t)0x1);
    if (Wire.available())
    {
      int portvalue = Wire.read();
      //portvalue += Wire.read()*256;
      return portvalue;
    }
  }
  return -1;
}

/*********************************************************************************************\
 * MCP23017 OUTPUT
\*********************************************************************************************/
boolean mcp23017(byte Par1, byte Par2)
{
  boolean success = false;
  byte portvalue = 0;
  byte unit = (Par1 - 1) / 16;
  byte port = Par1 - (unit * 16);
  uint8_t address = 0x20 + unit;
  byte IOBankConfigReg = 0;
  byte IOBankValueReg = 0x12;
  if (port > 8)
  {
    port = port - 8;
    IOBankConfigReg++;
    IOBankValueReg++;
  }
  // turn this port into output, first read current config
  Wire.beginTransmission(address);
  Wire.write(IOBankConfigReg); // IO config register
  Wire.endTransmission();
  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    portvalue = Wire.read();
    portvalue &= ~(1 << (port - 1)); // change pin from (default) input to output

    // write new IO config
    Wire.beginTransmission(address);
    Wire.write(IOBankConfigReg); // IO config register
    Wire.write(portvalue);
    Wire.endTransmission();
  }
  // get the current pin status
  Wire.beginTransmission(address);
  Wire.write(IOBankValueReg); // IO data register
  Wire.endTransmission();
  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    portvalue = Wire.read();
    if (Par2 == 1)
      portvalue |= (1 << (port - 1));
    else
      portvalue &= ~(1 << (port - 1));

    // write back new data
    Wire.beginTransmission(address);
    Wire.write(IOBankValueReg);
    Wire.write(portvalue);
    Wire.endTransmission();
    success = true;
  }
}

