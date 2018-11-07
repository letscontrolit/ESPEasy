#ifdef USES_P026
//#######################################################################################################
//#################################### Plugin 026: System Info ##########################################
//#######################################################################################################

#define PLUGIN_026
#define PLUGIN_ID_026         26
#define PLUGIN_NAME_026       "Generic - System Info"
#define PLUGIN_VALUENAME1_026 ""

boolean Plugin_026(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_026;
        Device[deviceCount].VType = SENSOR_TYPE_QUAD;
        Device[deviceCount].ValueCount = 4;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].FormulaOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_026);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_026));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME1_026));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME1_026));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME1_026));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice;
        String options[12];
        options[0] = F("Uptime");
        options[1] = F("Free RAM");
        options[2] = F("Wifi RSSI");
        options[3] = F("Input VCC");
        options[4] = F("System load");
        options[5] = F("IP 1.Octet");
        options[6] = F("IP 2.Octet");
        options[7] = F("IP 3.Octet");
        options[8] = F("IP 4.Octet");
        options[9] = F("Web activity");
        options[10] = F("Free Stack");
        options[11] = F("None");

        choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        addFormSelector(F("Indicator"), F("p026a"), 12, options, NULL, choice);
        choice = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        addFormSelector(F("Indicator"), F("p026b"), 12, options, NULL, choice);
        choice = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        addFormSelector(F("Indicator"), F("p026c"), 12, options, NULL, choice);
        choice = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
        addFormSelector(F("Indicator"), F("p026d"), 12, options, NULL, choice);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("p026a"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("p026b"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("p026c"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = getFormItemInt(F("p026d"));
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        UserVar[event->BaseVarIndex] = P026_get_value(Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        UserVar[event->BaseVarIndex+1] = P026_get_value(Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        UserVar[event->BaseVarIndex+2] = P026_get_value(Settings.TaskDevicePluginConfig[event->TaskIndex][2]);
        UserVar[event->BaseVarIndex+3] = P026_get_value(Settings.TaskDevicePluginConfig[event->TaskIndex][3]);
        if (loglevelActiveFor(LOG_LEVEL_INFO)){
          String log = F("SYS  : ");
          log += UserVar[event->BaseVarIndex];
          log +=',';
          log += UserVar[event->BaseVarIndex+1];
          log +=',';
          log += UserVar[event->BaseVarIndex+2];
          log +=',';
          log += UserVar[event->BaseVarIndex+3];
          addLog(LOG_LEVEL_INFO,log);
        }
        success = true;
        break;
      }
  }
  return success;
}

float P026_get_value(int type)
{
  float value = 0;
          switch(type)
        {
          case 0:
          {
            value = (wdcounter /2);
            break;
          }
          case 1:
          {
            value = ESP.getFreeHeap();
            break;
          }
          case 2:
          {
            value = WiFi.RSSI();
            break;
          }
          case 3:
          {
#if FEATURE_ADC_VCC
            value = vcc;
#else
            value = -1.0;
#endif
            break;
          }
          case 4:
          {
            value = getCPUload();
            break;
          }
          case 5:
          {
            value = WiFi.localIP()[0];
            break;
          }
          case 6:
          {
            value = WiFi.localIP()[1];
            break;
          }
          case 7:
          {
            value = WiFi.localIP()[2];
            break;
          }
          case 8:
          {
            value = WiFi.localIP()[3];
            break;
          }
          case 9:
          {
            value = (millis()-lastWeb)/1000; // respond in seconds
            break;
          }
          case 10:
          {
            value = getCurrentFreeStack();
            break;
          }
        }
 return value;
}

#endif // USES_P026
