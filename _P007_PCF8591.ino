//#######################################################################################################
//#################################### Plugin 007: ExtWiredAnalog #######################################
//#######################################################################################################

/*********************************************************************************************\
 * This plugin provides support for 4 extra analog inputs, using the PCF8591 (NXP/Philips)
 * Support            : www.esp8266.nu
 * Date               : Apr 2015
 * Compatibility      : R004
 * Syntax             : "ExtWiredAnalog <Par1:Port>, <Par2:Variable>"
 *********************************************************************************************
 * Technical description:
 *
 * De PCF8591 is a IO Expander chip that connects through the I2C bus
 * Basic I2C address = 0x48
 * Each chip has 4 analog inputs
 * This commando reads the analog input en stores the result into a variable
 \*********************************************************************************************/
#define PLUGIN_007
#define PLUGIN_ID_007         7
#define PLUGIN_NAME_007       "PCF8591 Analog input"
#define PLUGIN_VALUENAME1_007 "Analog"

boolean Plugin_007(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  static byte portValue = 0;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_007;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 4;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_007);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_007));
        break;
      }

    case PLUGIN_READ:
      {
        byte unit = (Settings.TaskDevicePort[event->TaskIndex] - 1) / 4;
        byte port = Settings.TaskDevicePort[event->TaskIndex] - (unit * 4);
        uint8_t address = 0x48 + unit;

        // get the current pin value
        Wire.beginTransmission(address);
        Wire.write(port - 1);
        Wire.endTransmission();

        Wire.requestFrom(address, (uint8_t)0x2);
        if (Wire.available())
        {
          Wire.read(); // Read older value first (stored in chip)
          UserVar[event->BaseVarIndex] = (float)Wire.read(); // now read actual value and store into Nodo var
          Serial.print(F("PCF  : Analog value: "));
          Serial.println(UserVar[event->BaseVarIndex]);
          success = true;
        }
        break;
      }
  }
  return success;
}
