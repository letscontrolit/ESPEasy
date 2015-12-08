//#######################################################################################################
//########################### Controller Plugin 002: Domoticz MQTT ######################################
//#######################################################################################################

#define CPLUGIN_002
#define CPLUGIN_ID_002         2
#define CPLUGIN_NAME_002       "Domoticz MQTT"

boolean CPlugin_002(byte function, struct EventStruct *event)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_002;
        strcpy_P(Protocol[protocolCount].Name, PSTR(CPLUGIN_NAME_002));
        Protocol[protocolCount].usesMQTT = true;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 1883;
        break;
      }

    case CPLUGIN_PROTOCOL_TEMPLATE:
      {
        strcpy_P(Settings.MQTTsubscribe, PSTR("domoticz/out"));
        strcpy_P(Settings.MQTTpublish, PSTR("domoticz/in"));
        break;
      }

    case CPLUGIN_PROTOCOL_RECV:
      {
        char json[512];
        json[0] = 0;
        event->String2.toCharArray(json, 512);

        StaticJsonBuffer<512> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(json);

        if (root.success())
        {
          long idx = root["idx"];
          float nvalue = root["nvalue"];
          long nvaluealt = root["nvalue"];
          const char* name = root["name"];
          const char* svalue = root["svalue"];
          const char* svalue1 = root["svalue1"];
          const char* svalue2 = root["svalue2"];
          const char* svalue3 = root["svalue3"];

          if (nvalue == 0)
            nvalue = nvaluealt;

  // Direct Serial is allowed here, since this is still in development, it does not even work....
    
          Serial.print(F("MQTT : idx="));
          Serial.print(idx);
          Serial.print(F(" name="));
          Serial.print(name);
          Serial.print(F(" nvalue="));
          Serial.print(nvalue);
          Serial.print(F(" svalue="));
          Serial.print(svalue);
          Serial.print(F(" svalue1="));
          Serial.print(svalue1);
          Serial.print(F(" svalue2="));
          Serial.println(svalue2);
          Serial.print(F(" svalue3="));
          Serial.println(svalue3);
        }
        else
          Serial.println(F("MQTT : json parse error"));
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        StaticJsonBuffer<200> jsonBuffer;

        JsonObject& root = jsonBuffer.createObject();

        root["idx"] = event->idx;

        String values;
        char str[80];

        switch (event->sensorType)
        {
          case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
            root["nvalue"] = 0;
            values = UserVar[event->BaseVarIndex];
            values.toCharArray(str, 80);
            root["svalue"] =  str;
            break;
          case SENSOR_TYPE_LONG:                      // single LONG value, stored in two floats (rfid tags)
            root["nvalue"] = 0;
            values = (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16);
            values.toCharArray(str, 80);
            root["svalue"] =  str;
            break;
          case SENSOR_TYPE_TEMP_HUM:                      // temp + hum + hum_stat, used for DHT11
            root["nvalue"] = 0;
            values  = UserVar[event->BaseVarIndex];
            values += ";";
            values += UserVar[event->BaseVarIndex + 1];
            values += ";0";
            values.toCharArray(str, 80);
            root["svalue"] =  str;
            break;
          case SENSOR_TYPE_TEMP_BARO:                      // temp + hum + hum_stat + bar + bar_fore, used for BMP085
            root["nvalue"] = 0;
            values  = UserVar[event->BaseVarIndex];
            values += ";0;0;";
            values += UserVar[event->BaseVarIndex + 1];
            values += ";0";
            values.toCharArray(str, 80);
            root["svalue"] =  str;
            break;
          case SENSOR_TYPE_SWITCH:
            root["command"] = "switchlight";
            if (UserVar[event->BaseVarIndex] == 0)
              root["switchcmd"] = "Off";
            else
              root["switchcmd"] = "On";
            break;
          case SENSOR_TYPE_DIMMER:
            root["command"] = "switchlight";
            if (UserVar[event->BaseVarIndex] == 0)
              root["switchcmd"] = "Off";
            else
              root["Set%20Level"] = UserVar[event->BaseVarIndex];
            break;
        }

        char json[256];
        root.printTo(json, sizeof(json));
        String log = F("MQTT : ");
        log += json;
        addLog(LOG_LEVEL_DEBUG, json);

        String pubname = Settings.MQTTpublish;
        pubname.replace("%sysname%", Settings.Name);
        pubname.replace("%tskname%", ExtraTaskSettings.TaskDeviceName);
        pubname.replace("%id%", String(event->idx));

        if (!MQTTclient.publish(pubname, json))
        {
          log = F("MQTT publish failed");
          addLog(LOG_LEVEL_DEBUG, json);
          MQTTConnect();
          connectionFailures++;
        }
        else if (connectionFailures)
          connectionFailures--;
        break;
      }

  }
  return success;
}

