//#######################################################################################################
//#################################### Plugin 012: Pro Mini Analog ######################################
//#######################################################################################################

#define PLUGIN_012
#define PLUGIN_ID_012     12
#define PLUGIN_NAME_012       "ProMini Extender Analog"
#define PLUGIN_VALUENAME1_012 "Analog"

boolean Plugin_012(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_012;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 6;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_012);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_012));
        break;
      }

    case PLUGIN_READ:
      {
        uint8_t address = 0x7f;
        Wire.beginTransmission(address);
        Wire.write(4); // ADC Read
        Wire.write(Settings.TaskDevicePort[event->TaskIndex]);
        Wire.write(0);
        Wire.write(0);
        Wire.endTransmission();
        delay(1);  // remote unit needs some time to do the adc stuff
        Wire.requestFrom(address, (uint8_t)0x1);
        if (Wire.available())
          UserVar[event->BaseVarIndex] = Wire.read();
        Serial.print(F("PMADC: Analog: "));
        Serial.println(UserVar[event->BaseVarIndex]);
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String tmpString  = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex)
          tmpString = tmpString.substring(0, argIndex);
        if (tmpString.equalsIgnoreCase("EXTPWM"))
        {
          success = true;
          uint8_t address = 0x7f;
          Wire.beginTransmission(address);
          Wire.write(3);
          Wire.write(event->Par1);
          Wire.write(event->Par2 & 0xff);
          Wire.write((event->Par2 >> 8));
          Wire.endTransmission();
          if (printToWeb)
          {
            printWebString += F("EXTPWM ");
            printWebString += event->Par1;
            printWebString += F(" Set to ");
            printWebString += event->Par2;
            printWebString += F("<BR>");
          }
        }
        break;
      }
  }
  return success;
}
