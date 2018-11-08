//********************************************************************************
// Interface for Sending to Controllers
//********************************************************************************
void sendData(struct EventStruct *event)
{
  START_TIMER;
  checkRAM(F("sendData"));
 LoadTaskSettings(event->TaskIndex);
  if (Settings.UseRules)
    createRuleEvents(event->TaskIndex);

  if (Settings.UseValueLogger && Settings.InitSPI && Settings.Pin_sd_cs >= 0)
    SendValueLogger(event->TaskIndex);

//  if (!Settings.TaskDeviceSendData[event->TaskIndex])
//    return false;

/*
  // Disabed for now, using buffers at controller side.
  if (Settings.MessageDelay != 0)
  {
    const long dif = timePassedSince(lastSend);
    if (dif > 0 && dif < static_cast<long>(Settings.MessageDelay))
    {
      uint16_t delayms = Settings.MessageDelay - dif;
      //this is logged nowhere else, so might as well disable it here also:
      // addLog(LOG_LEVEL_DEBUG_MORE, String(F("CTRL : Message delay (ms): "))+delayms);

     delayBackground(delayms);

      // unsigned long timer = millis() + delayms;
      // while (!timeOutReached(timer))
      //   backgroundtasks();
    }
  }
*/

  LoadTaskSettings(event->TaskIndex); // could have changed during background tasks.

  for (byte x=0; x < CONTROLLER_MAX; x++)
  {
    event->ControllerIndex = x;
    event->idx = Settings.TaskDeviceID[x][event->TaskIndex];
    if (Settings.TaskDeviceSendData[event->ControllerIndex][event->TaskIndex] &&
        Settings.ControllerEnabled[event->ControllerIndex] &&
        Settings.Protocol[event->ControllerIndex])
    {
      event->ProtocolIndex = getProtocolIndex(Settings.Protocol[event->ControllerIndex]);
      if (validUserVar(event)) {
        CPlugin_ptr[event->ProtocolIndex](CPLUGIN_PROTOCOL_SEND, event, dummyString);
      } else {
        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("Invalid value detected for controller ");
          String controllerName;
          CPlugin_ptr[event->ProtocolIndex](CPLUGIN_GET_DEVICENAME, event, controllerName);
          log += controllerName;
          addLog(LOG_LEVEL_DEBUG, log);
        }
      }
    }
  }

  // FIXME TD-er: This PLUGIN_EVENT_OUT seems to be unused.
  PluginCall(PLUGIN_EVENT_OUT, event, dummyString);
  lastSend = millis();
  STOP_TIMER(SEND_DATA_STATS);
}

boolean validUserVar(struct EventStruct *event) {
  byte valueCount = getValueCountFromSensorType(event->sensorType);
  for (int i = 0; i < valueCount; ++i) {
    const float f(UserVar[event->BaseVarIndex + i]);
    if (!isValidFloat(f)) return false;
  }
  return true;
}

/*********************************************************************************************\
 * Handle incoming MQTT messages
\*********************************************************************************************/
// handle MQTT messages
void callback(char* c_topic, byte* b_payload, unsigned int length) {
  // char log[256];
  char c_payload[384];

  statusLED(true);
  int enabledMqttController = firstEnabledMQTTController();
  if (enabledMqttController < 0) {
    addLog(LOG_LEVEL_ERROR, F("MQTT : No enabled MQTT controller"));
    return;
  }
  if ((length + 1) > sizeof(c_payload))
  {
    addLog(LOG_LEVEL_ERROR, F("MQTT : Ignored too big message"));
    return;
  }

  //convert payload to string, and 0 terminate
  strncpy(c_payload,(char*)b_payload,length);
  c_payload[length] = 0;

/*
  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    String log;
    log=F("MQTT : Topic: ");
    log+=c_topic;
    addLog(LOG_LEVEL_DEBUG_MORE, log);

    log=F("MQTT : Payload: ");
    log+=c_payload;
    addLog(LOG_LEVEL_DEBUG_MORE, log);
  }
  */

  // sprintf_P(log, PSTR("%s%s"), "MQTT : Topic: ", c_topic);
  // addLog(LOG_LEVEL_DEBUG, log);
  // sprintf_P(log, PSTR("%s%s"), "MQTT : Payload: ", c_payload);
  // addLog(LOG_LEVEL_DEBUG, log);

  struct EventStruct TempEvent;
  // TD-er: This one cannot set the TaskIndex, but that may seem to work out.... hopefully.
  TempEvent.String1 = c_topic;
  TempEvent.String2 = c_payload;
  byte ProtocolIndex = getProtocolIndex(Settings.Protocol[enabledMqttController]);
  schedule_controller_event_timer(ProtocolIndex, CPLUGIN_PROTOCOL_RECV, &TempEvent);
}


/*********************************************************************************************\
 * Connect to MQTT message broker
\*********************************************************************************************/
bool MQTTConnect(int controller_idx)
{
  ++mqtt_reconnect_count;
  MakeControllerSettings(ControllerSettings);
  LoadControllerSettings(controller_idx, ControllerSettings);
  if (!ControllerSettings.checkHostReachable(true))
    return false;
  if (MQTTclient.connected()) {
    MQTTclient.disconnect();
    updateMQTTclient_connected();
  }
  mqtt = WiFiClient(); // workaround see: https://github.com/esp8266/Arduino/issues/4497#issuecomment-373023864
  mqtt.setTimeout(ControllerSettings.ClientTimeout);
  MQTTclient.setClient(mqtt);
  if (ControllerSettings.UseDNS) {
    MQTTclient.setServer(ControllerSettings.getHost().c_str(), ControllerSettings.Port);
  } else {
    MQTTclient.setServer(ControllerSettings.getIP(), ControllerSettings.Port);
  }
  MQTTclient.setCallback(callback);

  // MQTT needs a unique clientname to subscribe to broker
  String clientid;
  if(Settings.MQTTUseUnitNameAsClientId){
    clientid = Settings.Name;
    if (Settings.Unit != 0) { // only append non-zero unit number
      clientid += '_';
      clientid += Settings.Unit;
    }
  }
  else{
    clientid = F("ESPClient_");
    clientid += WiFi.macAddress();
  }
  // Work-around for 'lost connections' to the MQTT broker.
  // If the broker thinks the connection is still alive, a reconnect from the
  // client will be refused.
  // To overcome this issue, append the number of reconnects to the client ID to
  // make it different from the previous one.
  if (wifi_reconnects >= 1) {
    clientid += '_';
    clientid += wifi_reconnects;
  }

  String LWTTopic = ControllerSettings.MQTTLwtTopic;
  if(LWTTopic.length() == 0)
  {
    LWTTopic = ControllerSettings.Subscribe;
    LWTTopic += F("/LWT");
  }
  LWTTopic.replace(F("/#"), F("/status"));
  parseSystemVariables(LWTTopic, false);

  String LWTMessageConnect = ControllerSettings.LWTMessageConnect;
  if(LWTMessageConnect.length() == 0){
    LWTMessageConnect = DEFAULT_MQTT_LWT_CONNECT_MESSAGE;
  }
  parseSystemVariables(LWTMessageConnect, false);

  String LWTMessageDisconnect = ControllerSettings.LWTMessageDisconnect;
  if(LWTMessageDisconnect.length() == 0){
    LWTMessageDisconnect = DEFAULT_MQTT_LWT_DISCONNECT_MESSAGE;
  }
  parseSystemVariables(LWTMessageDisconnect, false);

  boolean MQTTresult = false;
  uint8_t willQos = 0;
  boolean willRetain = true;

  if ((SecuritySettings.ControllerUser[controller_idx] != 0) && (SecuritySettings.ControllerPassword[controller_idx] != 0)) {
    MQTTresult = MQTTclient.connect(clientid.c_str(), SecuritySettings.ControllerUser[controller_idx], SecuritySettings.ControllerPassword[controller_idx],
                                    LWTTopic.c_str(), willQos, willRetain, LWTMessageDisconnect.c_str());
  } else {
    MQTTresult = MQTTclient.connect(clientid.c_str(), LWTTopic.c_str(), willQos, willRetain, LWTMessageDisconnect.c_str());
  }
  delay(0);

  if (!MQTTresult) {
    addLog(LOG_LEVEL_ERROR, F("MQTT : Failed to connect to broker"));
    MQTTclient.disconnect();
    updateMQTTclient_connected();
    return false;
  }
  MQTTclient_should_reconnect = false;
  String log = F("MQTT : Connected to broker with client ID: ");
  log += clientid;
  addLog(LOG_LEVEL_INFO, log);
  String subscribeTo = ControllerSettings.Subscribe;
  parseSystemVariables(subscribeTo, false);
  MQTTclient.subscribe(subscribeTo.c_str());
  log = F("Subscribed to: ");
  log += subscribeTo;
  addLog(LOG_LEVEL_INFO, log);

  if (MQTTclient.publish(LWTTopic.c_str(), LWTMessageConnect.c_str(), 1)) {
    updateMQTTclient_connected();
    statusLED(true);
    mqtt_reconnect_count = 0;
    return true; // end loop if succesfull
  }
  return false;
}


/*********************************************************************************************\
 * Check connection MQTT message broker
\*********************************************************************************************/
bool MQTTCheck(int controller_idx)
{
  if (!WiFiConnected(10)) {
    return false;
  }
  byte ProtocolIndex = getProtocolIndex(Settings.Protocol[controller_idx]);
  if (Protocol[ProtocolIndex].usesMQTT)
  {
    if (MQTTclient_should_reconnect || !MQTTclient.connected())
    {
      if (MQTTclient_should_reconnect) {
        addLog(LOG_LEVEL_ERROR, F("MQTT : Intentional reconnect"));
      } else {
        connectionFailures += 2;
      }
      return MQTTConnect(controller_idx);
    } else if (connectionFailures) {
      connectionFailures--;
    }
  }
  // When no MQTT protocol is enabled, all is fine.
  return true;
}


/*********************************************************************************************\
 * Send status info to request source
\*********************************************************************************************/

void SendStatus(byte source, String status)
{
  switch(source)
  {
    case VALUE_SOURCE_HTTP:
    case VALUE_SOURCE_WEB_FRONTEND:
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

boolean MQTTpublish(int controller_idx, const char* topic, const char* payload, boolean retained)
{
  const bool success = MQTTDelayHandler.addToQueue(MQTT_queue_element(controller_idx, topic, payload, retained));
  scheduleNextMQTTdelayQueue();
  return success;
}

void scheduleNextMQTTdelayQueue() {
  scheduleNextDelayQueue(TIMER_MQTT_DELAY_QUEUE, MQTTDelayHandler.getNextScheduleTime());
}

void processMQTTdelayQueue() {
  START_TIMER;
  MQTT_queue_element* element(MQTTDelayHandler.getNext());
  if (element == NULL) return;
  if (MQTTclient.publish(element->_topic.c_str(), element->_payload.c_str(), element->_retained)) {
    setIntervalTimerOverride(TIMER_MQTT, 10); // Make sure the MQTT is being processed as soon as possible.
    MQTTDelayHandler.markProcessed(true);
  } else {
    MQTTDelayHandler.markProcessed(false);
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("MQTT : process MQTT queue not published, ");
      log += MQTTDelayHandler.sendQueue.size();
      log += F(" items left in queue");
      addLog(LOG_LEVEL_DEBUG, log);
    }
  }
  scheduleNextMQTTdelayQueue();
  STOP_TIMER(MQTT_DELAY_QUEUE);
}

/*********************************************************************************************\
 * Send status info back to channel where request came from
\*********************************************************************************************/
void MQTTStatus(String& status)
{
  MakeControllerSettings(ControllerSettings);
  int enabledMqttController = firstEnabledMQTTController();
  if (enabledMqttController >= 0) {
    LoadControllerSettings(enabledMqttController, ControllerSettings);
    String pubname = ControllerSettings.Subscribe;
    pubname.replace(F("/#"), F("/status"));
    parseSystemVariables(pubname, false);
    MQTTpublish(enabledMqttController, pubname.c_str(), status.c_str(),Settings.MQTTRetainFlag);
  }
}
