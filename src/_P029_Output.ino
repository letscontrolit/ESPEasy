#ifdef USES_P029
//#######################################################################################################
//#################################### Plugin 029: Output ###############################################
//#######################################################################################################

#define PLUGIN_029
#define PLUGIN_ID_029         29
#define PLUGIN_NAME_029       "Output - Domoticz MQTT Helper"
#define PLUGIN_VALUENAME1_029 "Output"
boolean Plugin_029(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_029;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE; // FIXME TD-er: Does this need a pin? Seems not to be used
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_029);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_029));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        // We need the index of the controller we are: 0-CONTROLLER_MAX
        byte controllerNr = 0;
          for (byte i=0; i < CONTROLLER_MAX; i++)
          {
//            if (Settings.Protocol[i] == CPLUGIN_ID_002) { controllerNr = i; }   -> error: 'CPLUGIN_ID_002' was not declared in this scope
            if (Settings.Protocol[i] == 2) { controllerNr = i; }
          }

        addHtml(F("<TR><TD>IDX:<TD>"));
        String id = F("TDID");   //="taskdeviceid"
        id += controllerNr + 1;
        addNumericBox(id, Settings.TaskDeviceID[controllerNr][event->TaskIndex], 0, DOMOTICZ_MAX_IDX);
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        success = true;
        break;
      }
  }
  return success;
}
#endif // USES_P029
