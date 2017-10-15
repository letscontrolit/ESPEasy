//********************************************************************************
// Interface for Sending to Controllers
//********************************************************************************
void sendData(struct EventStruct *event)
{
  LoadTaskSettings(event->TaskIndex);
  if (Settings.UseRules)
    createRuleEvents(event->TaskIndex);

  if (Settings.GlobalSync && Settings.TaskDeviceGlobalSync[event->TaskIndex])
    SendUDPTaskData(0, event->TaskIndex, event->TaskIndex);

  if (Settings.UseValueLogger && Settings.InitSPI && Settings.Pin_sd_cs >= 0)
    SendValueLogger(event->TaskIndex);

//  if (!Settings.TaskDeviceSendData[event->TaskIndex])
//    return false;

  if (Settings.MessageDelay != 0)
  {
    uint16_t dif = millis() - lastSend;
    if (dif < Settings.MessageDelay)
    {
      uint16_t delayms = Settings.MessageDelay - dif;
      //this is logged nowhere else, so might as well disable it here also:
      // addLog(LOG_LEVEL_DEBUG_MORE, String(F("CTRL : Message delay (ms): "))+delayms);
      delayBackground(delayms);

      // unsigned long timer = millis() + delayms;
      // while (millis() < timer)
      //   backgroundtasks();
    }
  }

  LoadTaskSettings(event->TaskIndex); // could have changed during background tasks.

  for (byte x=0; x < CONTROLLER_MAX; x++)
  {
    event->ControllerIndex = x;
    event->idx = Settings.TaskDeviceID[x][event->TaskIndex];

    if (Settings.TaskDeviceSendData[event->ControllerIndex][event->TaskIndex] && Settings.ControllerEnabled[event->ControllerIndex] && Settings.Protocol[event->ControllerIndex])
    {
      event->ProtocolIndex = getProtocolIndex(Settings.Protocol[event->ControllerIndex]);
      CPlugin_ptr[event->ProtocolIndex](CPLUGIN_PROTOCOL_SEND, event, dummyString);
    }
  }

  PluginCall(PLUGIN_EVENT_OUT, event, dummyString);
  lastSend = millis();
}


/*********************************************************************************************\
 * Handle incoming MQTT messages
\*********************************************************************************************/
// handle MQTT messages
void callback(char* c_topic, byte* b_payload, unsigned int length) {
  // char log[256];
  char c_payload[384];

  statusLED(true);

  if (length>sizeof(c_payload)-1)
  {
    addLog(LOG_LEVEL_ERROR, F("MQTT : Ignored too big message"));
  }

  //convert payload to string, and 0 terminate
  strncpy(c_payload,(char*)b_payload,length);
  c_payload[length] = 0;

  String log;
  log=F("MQTT : Topic: ");
  log+=c_topic;
  addLog(LOG_LEVEL_DEBUG, log);

  log=F("MQTT : Payload: ");
  log+=c_payload;
  addLog(LOG_LEVEL_DEBUG, log);

  // sprintf_P(log, PSTR("%s%s"), "MQTT : Topic: ", c_topic);
  // addLog(LOG_LEVEL_DEBUG, log);
  // sprintf_P(log, PSTR("%s%s"), "MQTT : Payload: ", c_payload);
  // addLog(LOG_LEVEL_DEBUG, log);

  struct EventStruct TempEvent;
  TempEvent.String1 = c_topic;
  TempEvent.String2 = c_payload;
  byte ProtocolIndex = getProtocolIndex(Settings.Protocol[0]);
  CPlugin_ptr[ProtocolIndex](CPLUGIN_PROTOCOL_RECV, &TempEvent, dummyString);
}


/*********************************************************************************************\
 * Connect to MQTT message broker
\*********************************************************************************************/
void MQTTConnect()
{
  ControllerSettingsStruct ControllerSettings;
  LoadControllerSettings(0, (byte*)&ControllerSettings, sizeof(ControllerSettings)); // todo index is now fixed to 0

  IPAddress MQTTBrokerIP(ControllerSettings.IP);
  MQTTclient.setServer(MQTTBrokerIP, ControllerSettings.Port);
  MQTTclient.setCallback(callback);

  // MQTT needs a unique clientname to subscribe to broker
  String clientid = Settings.Name; // clientid = %sysname%
  if (Settings.Unit != 0) // set unit number to zero if don't want to add to clientid
  {
    clientid += Settings.Unit;
  }
  String subscribeTo = "";
  String LWTTopic = ControllerSettings.Subscribe;
  LWTTopic.replace(F("/#"), F("/status"));
  LWTTopic.replace(F("%sysname%"), Settings.Name);

  for (byte x = 1; x < 3; x++)
  {
    String log = "";
    boolean MQTTresult = false;

    if ((SecuritySettings.ControllerUser[0] != 0) && (SecuritySettings.ControllerPassword[0] != 0))
      MQTTresult = MQTTclient.connect(clientid.c_str(), SecuritySettings.ControllerUser[0], SecuritySettings.ControllerPassword[0], LWTTopic.c_str(), 0, 1, "Connection Lost");
    else
      MQTTresult = MQTTclient.connect(clientid.c_str(), LWTTopic.c_str(), 0, 1, "Connection Lost");

    if (MQTTresult)
    {
      log = F("MQTT : ClientID ");
      log += clientid;
      log += " connected to broker";
      addLog(LOG_LEVEL_INFO, log);
      subscribeTo = ControllerSettings.Subscribe;
      subscribeTo.replace(F("%sysname%"), Settings.Name);
      MQTTclient.subscribe(subscribeTo.c_str());
      log = F("Subscribed to: ");
      log += subscribeTo;
      addLog(LOG_LEVEL_INFO, log);

      MQTTclient.publish(LWTTopic.c_str(), "Connected", 1);

      statusLED(true);
      break; // end loop if succesfull
    }
    else
    {
      log = F("MQTT : ClientID ");
      log += clientid;
      log +=" failed to connected to broker";
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
  byte ProtocolIndex = getProtocolIndex(Settings.Protocol[0]);
  if (Protocol[ProtocolIndex].usesMQTT)
  {
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
  ControllerSettingsStruct ControllerSettings;
  LoadControllerSettings(0, (byte*)&ControllerSettings, sizeof(ControllerSettings)); // todo index is now fixed to 0

  String pubname = ControllerSettings.Subscribe;
  pubname.replace(F("/#"), F("/status"));
  pubname.replace(F("%sysname%"), Settings.Name);
  MQTTclient.publish(pubname.c_str(), status.c_str(),Settings.MQTTRetainFlag);
}
