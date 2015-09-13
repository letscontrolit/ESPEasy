//#######################################################################################################
//#################################### Plugin 002: Analog ###############################################
//#######################################################################################################

#define PLUGIN_002
#define PLUGIN_ID_002        2

boolean Plugin_002(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_002;
        strcpy(Device[deviceCount].Name, "Analog input");
        Device[deviceCount].Type = DEVICE_TYPE_ANALOG;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        strcpy(Device[deviceCount].ValueNames[0], "Analog");
        break;
      }

    case PLUGIN_COMMAND:
      {
        int value = analogRead(A0);
        UserVar[event->BaseVarIndex] = (float)value;
        Serial.print("ADC  : Analog value: ");
        Serial.println(value);
        success = true;
        break;
      }
  }
  return success;
}
