#ifdef USES_P026
//#######################################################################################################
//#################################### Plugin 026: System Info ##########################################
//#######################################################################################################


#define PLUGIN_026
#define PLUGIN_ID_026         26
#define PLUGIN_NAME_026       "Generic - System Info"

#define P026_SENSOR_TYPE_INDEX  VARS_PER_TASK

String Plugin_026_valuename(byte value_nr, bool displayString) {
  switch (value_nr) {
    case 0:  return displayString ? F("Uptime") : F("uptime");
    case 1:  return displayString ? F("Free RAM") : F("freeheap");
    case 2:  return displayString ? F("Wifi RSSI") : F("rssi");
    case 3:  return displayString ? F("Input VCC") : F("vcc");
    case 4:  return displayString ? F("System load") : F("load");
    case 5:  return displayString ? F("IP 1.Octet") : F("ip1");
    case 6:  return displayString ? F("IP 2.Octet") : F("ip2");
    case 7:  return displayString ? F("IP 3.Octet") : F("ip3");
    case 8:  return displayString ? F("IP 4.Octet") : F("ip4");
    case 9:  return displayString ? F("Web activity") : F("web");
    case 10: return displayString ? F("Free Stack") : F("freestack");
    case 11: return displayString ? F("None") : F("");
    default:
    break;
  }
  return "";
}

boolean Plugin_026(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_026;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].ValueCount = 1;
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
        for (byte i = 0; i < VARS_PER_TASK; ++i) {
          if ( i < getValueCountFromSensorType(PCONFIG(P026_SENSOR_TYPE_INDEX))) {
            byte choice = PCONFIG(i);
            safe_strncpy(
              ExtraTaskSettings.TaskDeviceValueNames[i],
              Plugin_026_valuename(choice, false),
              sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
          } else {
            ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
          }
        }
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        sensorTypeHelper_webformLoad_simple(event, P026_SENSOR_TYPE_INDEX);
        String options[12];
        for (byte i = 0; i < 12; ++i) {
          options[i] = Plugin_026_valuename(i, true);
        }
        String label;
        for (byte i = 0; i < getValueCountFromSensorType(PCONFIG(P026_SENSOR_TYPE_INDEX)); ++i) {
          byte choice = PCONFIG(i);
          label = F("Indicator ");
          label += (i+1);
          addFormSelector(label, PCONFIG_LABEL(i), 12, options, NULL, choice);
        }

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        for (int i = 0; i < getValueCountFromSensorType(PCONFIG(P026_SENSOR_TYPE_INDEX)); ++i) {
          pconfig_webformSave(event, i);
        }
        sensorTypeHelper_saveSensorType(event, P026_SENSOR_TYPE_INDEX);
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        bool allDefault = true;
        for (byte i = 0; i < VARS_PER_TASK; ++i) {
          if (PCONFIG(i) != 0) {
            allDefault = false;
          }
        }
        if (allDefault) {
          // Reset nr 2 .. 4 to "None"
          for (byte i = 1; i < VARS_PER_TASK; ++i) {
            PCONFIG(i) = 11; // "None"
          }
        }
        sensorTypeHelper_setSensorType(event, P026_SENSOR_TYPE_INDEX);
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        for (int i = 0; i < getValueCountFromSensorType(PCONFIG(P026_SENSOR_TYPE_INDEX)); ++i) {
          UserVar[event->BaseVarIndex + i] = P026_get_value(PCONFIG(i));
        }
        if (loglevelActiveFor(LOG_LEVEL_INFO)){
          String log = F("SYS  : ");
          for (int i = 0; i < getValueCountFromSensorType(PCONFIG(P026_SENSOR_TYPE_INDEX)); ++i) {
            if (i != 0) {
              log +=',';
            }
            log += UserVar[event->BaseVarIndex + i];
          }
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
