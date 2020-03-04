#include "src/DataStructs/ESPEasy_EventStruct.h"
#include "src/Globals/Device.h"
#include "src/Globals/Plugins.h"
#include "src/Globals/Protocol.h"
#include "src/Globals/CPlugins.h"
#include "ESPEasy_common.h"
#include "ESPEasy_fdwdecl.h"
#include "ESPEasy_plugindefs.h"

// ********************************************************************************

// Interface for Sending to Controllers
// ********************************************************************************
void sendData(struct EventStruct *event)
{
  START_TIMER;
  checkRAM(F("sendData"));
  LoadTaskSettings(event->TaskIndex);

  if (Settings.UseRules) {
    createRuleEvents(event);
  }

  if (Settings.UseValueLogger && Settings.InitSPI && (Settings.Pin_sd_cs >= 0)) {
    SendValueLogger(event->TaskIndex);
  }

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

  for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++)
  {
    event->ControllerIndex = x;
    event->idx             = Settings.TaskDeviceID[x][event->TaskIndex];

    if (Settings.TaskDeviceSendData[event->ControllerIndex][event->TaskIndex] &&
        Settings.ControllerEnabled[event->ControllerIndex] &&
        Settings.Protocol[event->ControllerIndex])
    {
      protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(event->ControllerIndex);

      if (validUserVar(event)) {
        String dummy;
        CPluginCall(ProtocolIndex, CPlugin::Function::CPLUGIN_PROTOCOL_SEND, event, dummy);
      }
#ifndef BUILD_NO_DEBUG
      else {
        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("Invalid value detected for controller ");
          log += getCPluginNameFromProtocolIndex(ProtocolIndex);
          addLog(LOG_LEVEL_DEBUG, log);
        }
      }
#endif // ifndef BUILD_NO_DEBUG
    }
  }

  // FIXME TD-er: This PLUGIN_EVENT_OUT seems to be unused.
  {
    String dummy;
    PluginCall(PLUGIN_EVENT_OUT, event, dummy);
  }
  lastSend = millis();
  STOP_TIMER(SEND_DATA_STATS);
}

bool validUserVar(struct EventStruct *event) {
  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);
  if (!validDeviceIndex(DeviceIndex)) return false;

  switch (Device[DeviceIndex].VType) {
    case SENSOR_TYPE_LONG:    return true;
    case SENSOR_TYPE_STRING:  return true; // FIXME TD-er: Must look at length of event->String2 ?
    default:
      break;
  }
  byte valueCount = getValueCountFromSensorType(event->sensorType);

  for (int i = 0; i < valueCount; ++i) {
    const float f(UserVar[event->BaseVarIndex + i]);

    if (!isValidFloat(f)) { return false; }
  }
  return true;
}

#ifdef USES_MQTT
/*********************************************************************************************\
* Handle incoming MQTT messages
\*********************************************************************************************/

// handle MQTT messages
void callback(char *c_topic, byte *b_payload, unsigned int length) {
  statusLED(true);
  controllerIndex_t enabledMqttController = firstEnabledMQTT_ControllerIndex();

  if (!validControllerIndex(enabledMqttController)) {
    addLog(LOG_LEVEL_ERROR, F("MQTT : No enabled MQTT controller"));
    return;
  }

  if (length > MQTT_MAX_PACKET_SIZE)
  {
    addLog(LOG_LEVEL_ERROR, F("MQTT : Ignored too big message"));
    return;
  }

  struct EventStruct TempEvent;

  // TD-er: This one cannot set the TaskIndex, but that may seem to work out.... hopefully.
  TempEvent.String1 = c_topic;
  TempEvent.String2.reserve(length);

  for (unsigned int i = 0; i < length; ++i) {
    char c = static_cast<char>(*(b_payload + i));
    TempEvent.String2 += c;
  }

  /*
     if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
      String log;
      log=F("MQTT : Topic: ");
      log+=c_topic;
      addLog(LOG_LEVEL_DEBUG_MORE, log);

      log=F("MQTT : Payload: ");
      log+=TempEvent.String2;
      addLog(LOG_LEVEL_DEBUG_MORE, log);
     }
   */

  protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(enabledMqttController);
  schedule_controller_event_timer(ProtocolIndex, CPlugin::Function::CPLUGIN_PROTOCOL_RECV, &TempEvent);
}

/*********************************************************************************************\
* Disconnect from MQTT message broker
\*********************************************************************************************/
void MQTTDisconnect()
{
  if (MQTTclient.connected()) {
    MQTTclient.disconnect();
    addLog(LOG_LEVEL_INFO, F("MQTT : Disconnected from broker"));
  }
  updateMQTTclient_connected();
}

/*********************************************************************************************\
* Connect to MQTT message broker
\*********************************************************************************************/
bool MQTTConnect(controllerIndex_t controller_idx)
{
  ++mqtt_reconnect_count;
  MakeControllerSettings(ControllerSettings);
  LoadControllerSettings(controller_idx, ControllerSettings);

  if (!ControllerSettings.checkHostReachable(true)) {
    return false;
  }

  if (MQTTclient.connected()) {
    MQTTclient.disconnect();
  }
  updateMQTTclient_connected();
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

  if (Settings.MQTTUseUnitNameAsClientId) {
    clientid = Settings.getHostname();
  }
  else {
    clientid  = F("ESPClient_");
    clientid += WiFi.macAddress();
  }
  clientid.replace(' ', '_'); // Make sure no spaces are present in the client ID

  if ((wifi_reconnects >= 1) && Settings.uniqueMQTTclientIdReconnect()) {
    // Work-around for 'lost connections' to the MQTT broker.
    // If the broker thinks the connection is still alive, a reconnect from the
    // client will be refused.
    // To overcome this issue, append the number of reconnects to the client ID to
    // make it different from the previous one.
    clientid += '_';
    clientid += wifi_reconnects;
  }

  String LWTTopic = ControllerSettings.MQTTLwtTopic;

  if (LWTTopic.length() == 0)
  {
    LWTTopic  = ControllerSettings.Subscribe;
    LWTTopic += F("/LWT");
  }
  LWTTopic.replace(F("/#"), F("/status"));
  parseSystemVariables(LWTTopic, false);

  String LWTMessageConnect = ControllerSettings.LWTMessageConnect;

  if (LWTMessageConnect.length() == 0) {
    LWTMessageConnect = F(DEFAULT_MQTT_LWT_CONNECT_MESSAGE);
  }
  parseSystemVariables(LWTMessageConnect, false);

  String LWTMessageDisconnect = ControllerSettings.LWTMessageDisconnect;

  if (LWTMessageDisconnect.length() == 0) {
    LWTMessageDisconnect = F(DEFAULT_MQTT_LWT_DISCONNECT_MESSAGE);
  }
  parseSystemVariables(LWTMessageDisconnect, false);

  bool MQTTresult   = false;
  uint8_t willQos      = 0;
  bool willRetain   = ControllerSettings.mqtt_willRetain() && ControllerSettings.mqtt_sendLWT();
  bool cleanSession = ControllerSettings.mqtt_cleanSession(); // As suggested here: https://github.com/knolleary/pubsubclient/issues/458#issuecomment-493875150

  if ((SecuritySettings.ControllerUser[controller_idx] != 0) && (SecuritySettings.ControllerPassword[controller_idx] != 0)) {
    MQTTresult =
      MQTTclient.connect(clientid.c_str(),
                         SecuritySettings.ControllerUser[controller_idx],
                         SecuritySettings.ControllerPassword[controller_idx],
                         ControllerSettings.mqtt_sendLWT() ? LWTTopic.c_str() : nullptr,
                         willQos,
                         willRetain,
                         ControllerSettings.mqtt_sendLWT() ? LWTMessageDisconnect.c_str() : nullptr,
                         cleanSession);
  } else {
    MQTTresult = MQTTclient.connect(clientid.c_str(),
                                    nullptr,
                                    nullptr,
                                    ControllerSettings.mqtt_sendLWT() ? LWTTopic.c_str() : nullptr,
                                    willQos,
                                    willRetain,
                                    ControllerSettings.mqtt_sendLWT() ? LWTMessageDisconnect.c_str() : nullptr,
                                    cleanSession);
  }
  delay(0);


  byte controller_number = Settings.Protocol[controller_idx];	
  count_connection_results(MQTTresult, F("MQTT : Broker "), controller_number, ControllerSettings);
  if (!MQTTresult) {
    MQTTclient.disconnect();
    updateMQTTclient_connected();
    return false;
  }
  String log = F("MQTT : Connected to broker with client ID: ");
  log += clientid;
  addLog(LOG_LEVEL_INFO, log);
  String subscribeTo = ControllerSettings.Subscribe;
  parseSystemVariables(subscribeTo, false);
  MQTTclient.subscribe(subscribeTo.c_str());
  log  = F("Subscribed to: ");
  log += subscribeTo;
  addLog(LOG_LEVEL_INFO, log);

  if (MQTTclient.publish(LWTTopic.c_str(), LWTMessageConnect.c_str(), 1)) {
    updateMQTTclient_connected();
    statusLED(true);
    mqtt_reconnect_count = 0;

    // call all installed controller to publish autodiscover data
    if (MQTTclient_should_reconnect) { CPluginCall(CPlugin::Function::CPLUGIN_GOT_CONNECTED, 0); }
    MQTTclient_should_reconnect = false;
    return true; // end loop if succesfull
  }
  return false;
}

/*********************************************************************************************\
* Check connection MQTT message broker
\*********************************************************************************************/
bool MQTTCheck(controllerIndex_t controller_idx)
{
  if (!WiFiConnected(10)) {
    return false;
  }
  protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(controller_idx);
  if (!validProtocolIndex(ProtocolIndex)) {
    return false;
  }

  if (Protocol[ProtocolIndex].usesMQTT)
  {
    if (MQTTclient_should_reconnect || !MQTTclient.connected())
    {
      if (MQTTclient_should_reconnect) {
        addLog(LOG_LEVEL_ERROR, F("MQTT : Intentional reconnect"));
      } 
      return MQTTConnect(controller_idx);
    }
  }

  // When no MQTT protocol is enabled, all is fine.
  return true;
}
#endif //USES_MQTT

/*********************************************************************************************\
* Send status info to request source
\*********************************************************************************************/
void SendStatusOnlyIfNeeded(byte eventSource, bool param1, uint32_t key, const String& param2, int16_t param3) {
  if (SourceNeedsStatusUpdate(eventSource)) {
    SendStatus(eventSource, getPinStateJSON(param1, key, param2, param3));
  }
}

bool SourceNeedsStatusUpdate(byte eventSource)
{
  switch (eventSource) {
    case VALUE_SOURCE_HTTP:
    case VALUE_SOURCE_SERIAL:
    case VALUE_SOURCE_MQTT:
    case VALUE_SOURCE_WEB_FRONTEND:
      return true;
  }
  return false;
}

void SendStatus(byte source, const String& status)
{
  switch (source)
  {
    case VALUE_SOURCE_HTTP:
    case VALUE_SOURCE_WEB_FRONTEND:

      if (printToWeb) {
        printWebString += status;
      }
      break;
#ifdef USES_MQTT
    case VALUE_SOURCE_MQTT:
      MQTTStatus(status);
      break;
#endif //USES_MQTT
    case VALUE_SOURCE_SERIAL:
      serialPrintln(status);
      break;
  }
}

#ifdef USES_MQTT
bool MQTTpublish(controllerIndex_t controller_idx, const char *topic, const char *payload, bool retained)
{
  {
    MQTT_queue_element dummy_element(MQTT_queue_element(controller_idx, "", "", retained));
    if (MQTTDelayHandler.queueFull(dummy_element)) {
      // The queue is full, try to make some room first.
      addLog(LOG_LEVEL_DEBUG, F("MQTT : Extra processMQTTdelayQueue()"));
      processMQTTdelayQueue();
    }
  }
  const bool success = MQTTDelayHandler.addToQueue(MQTT_queue_element(controller_idx, topic, payload, retained));
  scheduleNextMQTTdelayQueue();
  return success;
}

void scheduleNextMQTTdelayQueue() {
  scheduleNextDelayQueue(TIMER_MQTT_DELAY_QUEUE, MQTTDelayHandler.getNextScheduleTime());
}

void processMQTTdelayQueue() {
  START_TIMER;
  MQTT_queue_element *element(MQTTDelayHandler.getNext());

  if (element == NULL) { return; }

  if (MQTTclient.publish(element->_topic.c_str(), element->_payload.c_str(), element->_retained)) {
    if (connectionFailures > 0) {
      --connectionFailures;
    }
    MQTTDelayHandler.markProcessed(true);
  } else {
    MQTTDelayHandler.markProcessed(false);
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("MQTT : process MQTT queue not published, ");
      log += MQTTDelayHandler.sendQueue.size();
      log += F(" items left in queue");
      addLog(LOG_LEVEL_DEBUG, log);
    }
#endif // ifndef BUILD_NO_DEBUG
  }
  setIntervalTimerOverride(TIMER_MQTT, 10); // Make sure the MQTT is being processed as soon as possible.
  scheduleNextMQTTdelayQueue();
  STOP_TIMER(MQTT_DELAY_QUEUE);
}

/*********************************************************************************************\
* Send status info back to channel where request came from
\*********************************************************************************************/
void MQTTStatus(const String& status)
{
  controllerIndex_t enabledMqttController = firstEnabledMQTT_ControllerIndex();

  if (validControllerIndex(enabledMqttController)) {
    MakeControllerSettings(ControllerSettings);
    LoadControllerSettings(enabledMqttController, ControllerSettings);
    String pubname = ControllerSettings.Subscribe;
    pubname.replace(F("/#"), F("/status"));
    parseSystemVariables(pubname, false);
    MQTTpublish(enabledMqttController, pubname.c_str(), status.c_str(), Settings.MQTTRetainFlag);
  }
}
#endif //USES_MQTT



/*********************************************************************************************\
 * send all sensordata
\*********************************************************************************************/
// void SensorSendAll()
// {
//   for (taskIndex_t x = 0; x < TASKS_MAX; x++)
//   {
//     SensorSendTask(x);
//   }
// }


/*********************************************************************************************\
 * send specific sensor task data
\*********************************************************************************************/
void SensorSendTask(taskIndex_t TaskIndex)
{
  if (!validTaskIndex(TaskIndex)) return;
  checkRAM(F("SensorSendTask"));
  if (Settings.TaskDeviceEnabled[TaskIndex])
  {
    byte varIndex = TaskIndex * VARS_PER_TASK;

    bool success = false;
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(TaskIndex);
    if (!validDeviceIndex(DeviceIndex)) return;

    LoadTaskSettings(TaskIndex);

    struct EventStruct TempEvent;
    TempEvent.TaskIndex = TaskIndex;
    TempEvent.BaseVarIndex = varIndex;
    // TempEvent.idx = Settings.TaskDeviceID[TaskIndex]; todo check
    TempEvent.sensorType = Device[DeviceIndex].VType;

    float preValue[VARS_PER_TASK]; // store values before change, in case we need it in the formula
    for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
      preValue[varNr] = UserVar[varIndex + varNr];

    if(Settings.TaskDeviceDataFeed[TaskIndex] == 0)  // only read local connected sensorsfeeds
    {
      String dummy;
      success = PluginCall(PLUGIN_READ, &TempEvent, dummy);
    }
    else
      success = true;

    if (success)
    {
      START_TIMER;
      for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
      {
        if (ExtraTaskSettings.TaskDeviceFormula[varNr][0] != 0)
        {
          String formula = ExtraTaskSettings.TaskDeviceFormula[varNr];
          formula.replace(F("%pvalue%"), String(preValue[varNr]));
          formula.replace(F("%value%"), String(UserVar[varIndex + varNr]));
          float result = 0;
          byte error = Calculate(formula.c_str(), &result);
          if (error == 0)
            UserVar[varIndex + varNr] = result;
        }
      }
      STOP_TIMER(COMPUTE_FORMULA_STATS);
      sendData(&TempEvent);
    }
  }
}