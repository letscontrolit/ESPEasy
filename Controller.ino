//********************************************************************************
// Interface for Sending to Controllers
//********************************************************************************
boolean sendData(struct EventStruct *event)
{
  LoadTaskSettings(event->TaskIndex);
  if (Settings.UseRules)
    createRuleEvents(event->TaskIndex);

  if (Settings.GlobalSync && Settings.TaskDeviceGlobalSync[event->TaskIndex])
    SendUDPTaskData(0, event->TaskIndex, event->TaskIndex);

  if (!Settings.TaskDeviceSendData[event->TaskIndex])
    return false;

  if (Settings.MessageDelay != 0)
  {
    uint16_t dif = millis() - lastSend;
    if (dif < Settings.MessageDelay)
    {
      uint16_t delayms = Settings.MessageDelay - dif;
      char log[30];
      sprintf_P(log, PSTR("HTTP : Delay %u ms"), delayms);
      addLog(LOG_LEVEL_DEBUG_MORE, log);
      unsigned long timer = millis() + delayms;
      while (millis() < timer)
        backgroundtasks();
    }
  }

  LoadTaskSettings(event->TaskIndex); // could have changed during background tasks.

  if (Settings.Protocol)
  {
    byte ProtocolIndex = getProtocolIndex(Settings.Protocol);
    CPlugin_ptr[ProtocolIndex](CPLUGIN_PROTOCOL_SEND, event, dummyString);
  }
  PluginCall(PLUGIN_EVENT_OUT, event, dummyString);
  lastSend = millis();
}


/*********************************************************************************************\
 * Handle incoming MQTT messages
\*********************************************************************************************/
// handle MQTT messages
void callback(char* c_topic, byte* b_payload, unsigned int length) {
  char log[128];
  char c_payload[128];
  strncpy(c_payload,(char*)b_payload,length);
  c_payload[length] = 0;
  statusLED(true);

  sprintf_P(log, PSTR("%s%s"), "MQTT : Topic: ", c_topic);
  addLog(LOG_LEVEL_DEBUG, log);
  sprintf_P(log, PSTR("%s%s"), "MQTT : Payload: ", c_payload);
  addLog(LOG_LEVEL_DEBUG, log);

  struct EventStruct TempEvent;
  TempEvent.String1 = c_topic;
  TempEvent.String2 = c_payload;
  byte ProtocolIndex = getProtocolIndex(Settings.Protocol);
  CPlugin_ptr[ProtocolIndex](CPLUGIN_PROTOCOL_RECV, &TempEvent, dummyString);
}


/*********************************************************************************************\
 * Connect to MQTT message broker
\*********************************************************************************************/
void MQTTConnect()
{
  IPAddress MQTTBrokerIP(Settings.Controller_IP);
  MQTTclient.setServer(MQTTBrokerIP, Settings.ControllerPort);
  MQTTclient.setCallback(callback);

  // MQTT needs a unique clientname to subscribe to broker
  String clientid = "ESPClient";
  clientid += Settings.Unit;
  String subscribeTo = "";

  String LWTTopic = Settings.MQTTsubscribe;
  LWTTopic.replace("/#", "/status");
  LWTTopic.replace("%sysname%", Settings.Name);
  
  for (byte x = 1; x < 3; x++)
  {
    String log = "";
    boolean MQTTresult = false;

    //boolean connect(const char* id);
    //boolean connect(const char* id, const char* user, const char* pass);
    //boolean connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
    //boolean connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);

    if ((SecuritySettings.ControllerUser[0] != 0) && (SecuritySettings.ControllerPassword[0] != 0))
      MQTTresult = MQTTclient.connect(clientid.c_str(), SecuritySettings.ControllerUser, SecuritySettings.ControllerPassword, LWTTopic.c_str(), 0, 0, "Connection Lost");
    else
      MQTTresult = MQTTclient.connect(clientid.c_str(), LWTTopic.c_str(), 0, 0, "Connection Lost");

    if (MQTTresult)
    {
      log = F("MQTT : Connected to broker");
      addLog(LOG_LEVEL_INFO, log);
      subscribeTo = Settings.MQTTsubscribe;
      subscribeTo.replace("%sysname%", Settings.Name);
      MQTTclient.subscribe(subscribeTo.c_str());
      log = F("Subscribed to: ");
      log += subscribeTo;
      addLog(LOG_LEVEL_INFO, log);
      break; // end loop if succesfull
    }
    else
    {
      log = F("MQTT : Failed to connected to broker");
      addLog(LOG_LEVEL_ERROR, log);
    }

    delay(500);
  }
}


/*********************************************************************************************\
 * Check connection MQTT message broker
\*********************************************************************************************/
void MQTTCheck()
{
  byte ProtocolIndex = getProtocolIndex(Settings.Protocol);
  if (Protocol[ProtocolIndex].usesMQTT)
    if (!MQTTclient.connected())
    {
      String log = F("MQTT : Connection lost");
      addLog(LOG_LEVEL_ERROR, log);
      connectionFailures += 2;
      MQTTclient.disconnect();
      delay(1000);
      MQTTConnect();
    }
    else if (connectionFailures)
      connectionFailures--;
}


/*********************************************************************************************\
 * Send status info to request source
\*********************************************************************************************/

void SendStatus(byte source, String status)
{
  switch(source)
  {
    case VALUE_SOURCE_HTTP:
      if (printToWeb)
        printWebString += status;
      break;
    case VALUE_SOURCE_MQTT:
      MQTTStatus(status);
      break;
    case VALUE_SOURCE_SERIAL:
      Serial.println(status);
      break;
  }
}


/*********************************************************************************************\
 * Send status info back to channel where request came from
\*********************************************************************************************/
void MQTTStatus(String& status)
{
  String pubname = Settings.MQTTsubscribe;
  pubname.replace("/#", "/status");
  pubname.replace("%sysname%", Settings.Name);
  MQTTclient.publish(pubname.c_str(), status.c_str());
}



