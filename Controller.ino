//********************************************************************************
// Interface for Sending to Controllers
//********************************************************************************
boolean sendData(struct EventStruct *event)
{
  LoadTaskSettings(event->TaskIndex);
  if (Settings.UseRules)
  {
    byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[event->TaskIndex]);
    for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
    {
      LoadTaskSettings(event->TaskIndex);
      String eventString = ExtraTaskSettings.TaskDeviceName;
      eventString += F("#");
      eventString += ExtraTaskSettings.TaskDeviceValueNames[varNr];
      eventString += F("=");
      
      if (event->sensorType == SENSOR_TYPE_LONG)
        eventString += (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16);
      else
        eventString += UserVar[event->BaseVarIndex + varNr];
      
      rulesProcessing(eventString);
    }
  }

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

  if (Settings.TaskDeviceGlobalSync[event->TaskIndex])
    SendUDPTaskData(0, event->TaskIndex, event->TaskIndex);

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
void callback(const MQTT::Publish& pub) {
  char log[80];
  char tmp[80];
  String topic = pub.topic();
  String payload = pub.payload_string();

  statusLED(true);

  topic.toCharArray(tmp, 80);
  sprintf_P(log, PSTR("%s%s"), "MQTT : Topic: ", tmp);
  addLog(LOG_LEVEL_DEBUG, log);
  payload.toCharArray(tmp, 80);
  sprintf_P(log, PSTR("%s%s"), "MQTT : Payload: ", tmp);
  addLog(LOG_LEVEL_DEBUG, log);

  struct EventStruct TempEvent;
  TempEvent.String1 = topic;
  TempEvent.String2 = payload;
  byte ProtocolIndex = getProtocolIndex(Settings.Protocol);
  CPlugin_ptr[ProtocolIndex](CPLUGIN_PROTOCOL_RECV, &TempEvent, dummyString);
}


/*********************************************************************************************\
 * Connect to MQTT message broker
\*********************************************************************************************/
void MQTTConnect()
{
  IPAddress MQTTBrokerIP(Settings.Controller_IP);
  MQTTclient.set_server(MQTTBrokerIP, Settings.ControllerPort);
  MQTTclient.set_callback(callback);

  // MQTT needs a unique clientname to subscribe to broker
  String clientid = "ESPClient";
  clientid += Settings.Unit;
  String subscribeTo = "";

  for (byte x = 1; x < 3; x++)
  {
    String log = "";
    boolean MQTTresult = false;

    if ((SecuritySettings.ControllerUser) && (SecuritySettings.ControllerPassword))
      MQTTresult = (MQTTclient.connect(MQTT::Connect(clientid).set_auth(SecuritySettings.ControllerUser, SecuritySettings.ControllerPassword)));
    else
      MQTTresult = (MQTTclient.connect(clientid));

    if (MQTTresult)
    {
      log = F("MQTT : Connected to broker");
      addLog(LOG_LEVEL_INFO, log);
      subscribeTo = Settings.MQTTsubscribe;
      subscribeTo.replace("%sysname%", Settings.Name);
      MQTTclient.subscribe(subscribeTo);
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


struct NodeStruct
{
  byte ip[4];
  byte age;
} Nodes[UNIT_MAX];



