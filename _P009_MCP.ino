//#######################################################################################################
//#################################### Plugin 009: MCP23017 input #######################################
//#######################################################################################################

#define PLUGIN_009
#define PLUGIN_ID_009         9
#define PLUGIN_NAME_009       "MCP23017 input"
#define PLUGIN_VALUENAME1_009 "Switch"

boolean Plugin_009(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_009;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 16;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_009);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_009));
        break;
      }

    case PLUGIN_COMMAND:
      {
        byte Par1 = Settings.TaskDevicePort[event->TaskIndex];
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
        // get the current pin status
        Wire.beginTransmission(address);
        Wire.write(IOBankValueReg); // IO data register
        Wire.endTransmission();
        Wire.requestFrom(address, (uint8_t)0x1);
        if (Wire.available())
        {
          portvalue = ((Wire.read() & _BV(port - 1)) >> (port - 1));
          UserVar[event->BaseVarIndex] = (float)portvalue;
          Serial.print(F("MCP  : Input Value : "));
          Serial.println(UserVar[event->BaseVarIndex]);
          success = true;
        }
        break;
      }
  }
  return success;
}
