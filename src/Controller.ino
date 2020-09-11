#include "ESPEasy_common.h"
#include "ESPEasy_fdwdecl.h"
#include "ESPEasy_plugindefs.h"
#include "src/ControllerQueue/MQTT_queue_element.h"
#include "src/DataStructs/ControllerSettingsStruct.h"
#include "src/DataStructs/ESPEasy_EventStruct.h"
#include "src/Globals/CPlugins.h"
#include "src/Globals/Device.h"
#include "src/Globals/ESPEasy_Scheduler.h"
#include "src/Globals/MQTT.h"
#include "src/Globals/Plugins.h"
#include "src/Globals/Protocol.h"
#include "_CPlugin_Helper.h"

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

  if (Settings.UseValueLogger && Settings.InitSPI>0 && (Settings.Pin_sd_cs >= 0)) {
    SendValueLogger(event->TaskIndex);
  }

  LoadTaskSettings(event->TaskIndex); // could have changed during background tasks.
  if (event->sensorType == SENSOR_TYPE_NONE) {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);
    if (validDeviceIndex(DeviceIndex)) {
      event->sensorType = Device[DeviceIndex].VType;
    }
  }

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
  switch (event->sensorType) {
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

  // TD-er: This one cannot set the TaskIndex, but that may seem to work out.... hopefully.
  protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(enabledMqttController);
  Scheduler.schedule_mqtt_controller_event_timer(
    ProtocolIndex, 
    CPlugin::Function::CPLUGIN_PROTOCOL_RECV,
    c_topic, b_payload, length);
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
  if (!AllocatedControllerSettings()) {
    addLog(LOG_LEVEL_ERROR, F("MQTT : Cannot connect, out of RAM"));
    return false;
  }
  LoadControllerSettings(controller_idx, ControllerSettings);

  if (!ControllerSettings.checkHostReachable(true)) {
    return false;
  }

  if (MQTTclient.connected()) {
    MQTTclient.disconnect();
  }
  updateMQTTclient_connected();
//  mqtt = WiFiClient(); // workaround see: https://github.com/esp8266/Arduino/issues/4497#issuecomment-373023864
  yield();
  mqtt.setTimeout(ControllerSettings.ClientTimeout);
  MQTTclient.setClient(mqtt);

  if (ControllerSettings.UseDNS) {
    MQTTclient.setServer(ControllerSettings.getHost().c_str(), ControllerSettings.Port);
  } else {
    MQTTclient.setServer(ControllerSettings.getIP(), ControllerSettings.Port);
  }
  MQTTclient.setCallback(callback);

  // MQTT needs a unique clientname to subscribe to broker
  String clientid = getMQTTclientID(ControllerSettings);

  String  LWTTopic             = getLWT_topic(ControllerSettings);
  String  LWTMessageDisconnect = getLWT_messageDisconnect(ControllerSettings);
  bool    MQTTresult           = false;
  uint8_t willQos              = 0;
  bool    willRetain           = ControllerSettings.mqtt_willRetain() && ControllerSettings.mqtt_sendLWT();
  bool    cleanSession         = ControllerSettings.mqtt_cleanSession(); // As suggested here:
                                                                         // https://github.com/knolleary/pubsubclient/issues/458#issuecomment-493875150

  if (hasControllerCredentialsSet(controller_idx, ControllerSettings)) {
    MQTTresult =
      MQTTclient.connect(clientid.c_str(),
                         getControllerUser(controller_idx, ControllerSettings).c_str(),
                         getControllerPass(controller_idx, ControllerSettings).c_str(),
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
  count_connection_results(MQTTresult, F("MQTT : Broker "), controller_number);

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

  updateMQTTclient_connected();
  statusLED(true);
  mqtt_reconnect_count = 0;

  // call all installed controller to publish autodiscover data
  if (MQTTclient_should_reconnect) { CPluginCall(CPlugin::Function::CPLUGIN_GOT_CONNECTED, 0); }
  MQTTclient_should_reconnect = false;

  if (ControllerSettings.mqtt_sendLWT()) {
    String LWTMessageConnect = getLWT_messageConnect(ControllerSettings);

    if (!MQTTclient.publish(LWTTopic.c_str(), LWTMessageConnect.c_str(), willRetain)) {
      MQTTclient_must_send_LWT_connected = true;
    }
  }

  return true;
}

String getMQTTclientID(const ControllerSettingsStruct& ControllerSettings) {
  String clientid = ControllerSettings.ClientID;

  if (clientid.length() == 0) {
    // Try to generate some default
    clientid = F(CONTROLLER_DEFAULT_CLIENTID);
  }
  parseSystemVariables(clientid, false);
  clientid.replace(' ', '_'); // Make sure no spaces are present in the client ID

  if ((wifi_reconnects >= 1) && ControllerSettings.mqtt_uniqueMQTTclientIdReconnect()) {
    // Work-around for 'lost connections' to the MQTT broker.
    // If the broker thinks the connection is still alive, a reconnect from the
    // client will be refused.
    // To overcome this issue, append the number of reconnects to the client ID to
    // make it different from the previous one.
    clientid += '_';
    clientid += wifi_reconnects;
  }
  return clientid;
}

/*********************************************************************************************\
* Check connection MQTT message broker
\*********************************************************************************************/
bool MQTTCheck(controllerIndex_t controller_idx)
{
  if (!NetworkConnected(10)) {
    return false;
  }
  protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(controller_idx);

  if (!validProtocolIndex(ProtocolIndex)) {
    return false;
  }

  if (Protocol[ProtocolIndex].usesMQTT)
  {
    bool mqtt_sendLWT = false;
    String LWTTopic, LWTMessageConnect;
    bool willRetain = false;
    {
      MakeControllerSettings(ControllerSettings);
      if (!AllocatedControllerSettings()) {
        addLog(LOG_LEVEL_ERROR, F("MQTT : Cannot check, out of RAM"));
        return false;
      }

      LoadControllerSettings(controller_idx, ControllerSettings);

      // FIXME TD-er: Is this still needed?
      /*
      #ifdef USES_ESPEASY_NOW
      if (!MQTTclient.connected()) {
        if (ControllerSettings.enableESPEasyNowFallback()) {
          return true;
        }
      }
      #endif
      */

      if (!ControllerSettings.isSet()) {
        return true;
      }

      if (ControllerSettings.mqtt_sendLWT()) {
        mqtt_sendLWT = true;
        LWTTopic          = getLWT_topic(ControllerSettings);
        LWTMessageConnect = getLWT_messageConnect(ControllerSettings);
        willRetain        = ControllerSettings.mqtt_willRetain();
      }
    }
    if (MQTTclient_should_reconnect || !MQTTclient.connected())
    {
      if (MQTTclient_should_reconnect) {
        addLog(LOG_LEVEL_ERROR, F("MQTT : Intentional reconnect"));
      }
      return MQTTConnect(controller_idx);
    }

    if (MQTTclient_must_send_LWT_connected) {
      if (mqtt_sendLWT) {
        if (MQTTclient.publish(LWTTopic.c_str(), LWTMessageConnect.c_str(), willRetain)) {
          MQTTclient_must_send_LWT_connected = false;
        }
      } else {
        MQTTclient_must_send_LWT_connected = false;
      }
    }
  }

  // When no MQTT protocol is enabled, all is fine.
  return true;
}


String getLWT_topic(const ControllerSettingsStruct& ControllerSettings) {
  String LWTTopic;

  if (ControllerSettings.mqtt_sendLWT()) {
    LWTTopic = ControllerSettings.MQTTLwtTopic;

    if (LWTTopic.length() == 0)
    {
      LWTTopic  = ControllerSettings.Subscribe;
      LWTTopic += F("/LWT");
    }
    LWTTopic.replace(F("/#"), F("/status"));
    parseSystemVariables(LWTTopic, false);
  }
  return LWTTopic;
}

String getLWT_messageConnect(const ControllerSettingsStruct& ControllerSettings) {
  String LWTMessageConnect;

  if (ControllerSettings.mqtt_sendLWT()) {
    LWTMessageConnect = ControllerSettings.LWTMessageConnect;

    if (LWTMessageConnect.length() == 0) {
      LWTMessageConnect = F(DEFAULT_MQTT_LWT_CONNECT_MESSAGE);
    }
    parseSystemVariables(LWTMessageConnect, false);
  }
  return LWTMessageConnect;
}

String getLWT_messageDisconnect(const ControllerSettingsStruct& ControllerSettings) {
  String LWTMessageDisconnect;

  if (ControllerSettings.mqtt_sendLWT()) {
    LWTMessageDisconnect = ControllerSettings.LWTMessageDisconnect;

    if (LWTMessageDisconnect.length() == 0) {
      LWTMessageDisconnect = F(DEFAULT_MQTT_LWT_DISCONNECT_MESSAGE);
    }
    parseSystemVariables(LWTMessageDisconnect, false);
  }
  return LWTMessageDisconnect;
}

#endif // USES_MQTT

/*********************************************************************************************\
* Send status info to request source
\*********************************************************************************************/
void SendStatusOnlyIfNeeded(EventValueSource::Enum eventSource, bool param1, uint32_t key, const String& param2, int16_t param3) {
  if (SourceNeedsStatusUpdate(eventSource)) {
    SendStatus(eventSource, getPinStateJSON(param1, key, param2, param3));
  }
}

bool SourceNeedsStatusUpdate(EventValueSource::Enum eventSource)
{
  switch (eventSource) {
    case EventValueSource::Enum::VALUE_SOURCE_HTTP:
    case EventValueSource::Enum::VALUE_SOURCE_SERIAL:
    case EventValueSource::Enum::VALUE_SOURCE_MQTT:
    case EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND:
      return true;

    default: 
      break;
  }
  return false;
}

void SendStatus(EventValueSource::Enum source, const String& status)
{
  switch (source)
  {
    case EventValueSource::Enum::VALUE_SOURCE_HTTP:
    case EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND:

      if (printToWeb) {
        printWebString += status;
      }
      break;
#ifdef USES_MQTT
    case EventValueSource::Enum::VALUE_SOURCE_MQTT:
      MQTTStatus(status);
      break;
#endif //USES_MQTT
    case EventValueSource::Enum::VALUE_SOURCE_SERIAL:
      serialPrintln(status);
      break;

    default: 
      break;
  }
}

#ifdef USES_MQTT
bool MQTT_queueFull(controllerIndex_t controller_idx) {
  if (MQTTDelayHandler == nullptr) {
    return true;
  }
  MQTT_queue_element dummy_element;
  dummy_element.controller_idx = controller_idx;
  if (MQTTDelayHandler->queueFull(dummy_element)) {
    // The queue is full, try to make some room first.
    processMQTTdelayQueue();
    return MQTTDelayHandler->queueFull(dummy_element);
  }
  return false;
}

bool MQTTpublish(controllerIndex_t controller_idx, const char *topic, const char *payload, bool retained)
{
  if (MQTTDelayHandler == nullptr) {
    return false;
  }
  if (MQTT_queueFull(controller_idx)) {
    return false;
  }
  const bool success = MQTTDelayHandler->addToQueue(MQTT_queue_element(controller_idx, topic, payload, retained));
  scheduleNextMQTTdelayQueue();
  return success;
}


/*********************************************************************************************\
* Send status info back to channel where request came from
\*********************************************************************************************/
void MQTTStatus(const String& status)
{
  controllerIndex_t enabledMqttController = firstEnabledMQTT_ControllerIndex();

  if (validControllerIndex(enabledMqttController)) {
    String pubname;
    bool mqtt_retainFlag;
    {
      // Place the ControllerSettings in a scope to free the memory as soon as we got all relevant information.
      MakeControllerSettings(ControllerSettings);
      if (!AllocatedControllerSettings()) {
        addLog(LOG_LEVEL_ERROR, F("MQTT : Cannot send status, out of RAM"));
        return;
      }

      LoadControllerSettings(enabledMqttController, ControllerSettings);
      pubname = ControllerSettings.Publish;
      mqtt_retainFlag = ControllerSettings.mqtt_retainFlag();
    }

    pubname.replace(F("/#"), F("/status"));
    parseSystemVariables(pubname, false);
    MQTTpublish(enabledMqttController, pubname.c_str(), status.c_str(), mqtt_retainFlag);
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
 * send specific sensor task data, effectively calling PluginCall(PLUGIN_READ...)
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
      if (Device[DeviceIndex].FormulaOption) {
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
      }
      sendData(&TempEvent);
    }
  }
}