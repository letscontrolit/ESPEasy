#include "../ESPEasyCore/Controller.h"

#include "../../ESPEasy_common.h"
#include "../../ESPEasy-Globals.h"

#include "../../_Plugin_Helper.h"

#include "../ControllerQueue/MQTT_queue_element.h"

#include "../CustomBuild/Certificate_CA.h"

#include "../DataStructs/ControllerSettingsStruct.h"
#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../DataTypes/SPI_options.h"

#include "../ESPEasyCore/ESPEasyRules.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/CPlugins.h"
#include "../Globals/Device.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/MQTT.h"
#include "../Globals/Plugins.h"
#include "../Globals/RulesCalculate.h"

#include "../Helpers/_CPlugin_Helper.h"

// #include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Network.h"
#include "../Helpers/PeriodicalActions.h"
#include "../Helpers/PortStatus.h"


constexpr pluginID_t PLUGIN_ID_MQTT_IMPORT(37);

// ********************************************************************************
// Interface for Sending to Controllers
// ********************************************************************************
void sendData(struct EventStruct *event, bool sendEvents)
{
  START_TIMER;
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("sendData"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  //  LoadTaskSettings(event->TaskIndex);

  if (Settings.UseRules && sendEvents) {
    createRuleEvents(event);
  }

  if (Settings.UseValueLogger && (Settings.InitSPI > static_cast<int>(SPI_Options_e::None)) && (Settings.Pin_sd_cs >= 0)) {
    SendValueLogger(event->TaskIndex);
  }

  //  LoadTaskSettings(event->TaskIndex); // could have changed during background tasks.

  for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++)
  {
    if (Settings.ControllerEnabled[x] &&
        Settings.TaskDeviceSendData[x][event->TaskIndex] &&
        Settings.Protocol[x])
    {
      event->ControllerIndex = x;
      const protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(event->ControllerIndex);
      event->idx = Settings.TaskDeviceID[x][event->TaskIndex];

      if (validUserVar(event)) {
        String dummy;
        CPluginCall(ProtocolIndex, CPlugin::Function::CPLUGIN_PROTOCOL_SEND, event, dummy);
      }
#ifndef BUILD_NO_DEBUG
      else {
        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("Invalid value detected for controller ");
          log += getCPluginNameFromProtocolIndex(ProtocolIndex);
          addLogMove(LOG_LEVEL_DEBUG, log);
        }
      }
#endif // ifndef BUILD_NO_DEBUG
    }
  }

  lastSend = millis();
  STOP_TIMER(SEND_DATA_STATS);
}

bool validUserVar(struct EventStruct *event) {
  if (!validTaskIndex(event->TaskIndex)) { return false; }
  const Sensor_VType vtype = event->getSensorType();

  if (isIntegerOutputDataType(vtype) ||
      (vtype == Sensor_VType::SENSOR_TYPE_STRING)) // FIXME TD-er: Must look at length of event->String2 ?
  {
    return true;
  }
  const uint8_t valueCount = getValueCountForTask(event->TaskIndex);

  for (int i = 0; i < valueCount; ++i) {
    if (!UserVar.isValid(event->TaskIndex, i, vtype)) {
      return false;
    }
  }
  return true;
}

#if FEATURE_MQTT

/*********************************************************************************************\
* Handle incoming MQTT messages
\*********************************************************************************************/

// handle MQTT messages
void incoming_mqtt_callback(char *c_topic, uint8_t *b_payload, unsigned int length) {
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

  deviceIndex_t DeviceIndex = getDeviceIndex(PLUGIN_ID_MQTT_IMPORT); // Check if P037_MQTTimport is present in the build

  if (validDeviceIndex(DeviceIndex)) {
    //  Here we loop over all tasks and call each 037 plugin with function PLUGIN_MQTT_IMPORT
    for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; taskIndex++)
    {
      if (Settings.TaskDeviceEnabled[taskIndex] && (Settings.getPluginID_for_task(taskIndex) == PLUGIN_ID_MQTT_IMPORT))
      {
        Scheduler.schedule_mqtt_plugin_import_event_timer(
          DeviceIndex, taskIndex, PLUGIN_MQTT_IMPORT,
          c_topic, b_payload, length);
      }
    }
  }
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
  if (MQTTclient_next_connect_attempt.isSet() && !MQTTclient_next_connect_attempt.timeoutReached(timermqtt_interval)) {
    return false;
  }
  MQTTclient_next_connect_attempt.setNow();
  ++mqtt_reconnect_count;

  MakeControllerSettings(ControllerSettings); // -V522

  if (!AllocatedControllerSettings()) {
    addLog(LOG_LEVEL_ERROR, F("MQTT : Cannot connect, out of RAM"));
    return false;
  }
  LoadControllerSettings(controller_idx, *ControllerSettings);

  if (!ControllerSettings->checkHostReachable(true)) {
    return false;
  }

  if (MQTTclient.connected()) {
    MQTTclient.disconnect();
    # if FEATURE_MQTT_TLS

    if (mqtt_tls != nullptr) {
      delete mqtt_tls;
      mqtt_tls = nullptr;
    }
    mqtt_rootCA.clear();
    # endif // if FEATURE_MQTT_TLS
  }

  updateMQTTclient_connected();

  //  mqtt = WiFiClient(); // workaround see: https://github.com/esp8266/Arduino/issues/4497#issuecomment-373023864
  delay(0);
# if FEATURE_MQTT_TLS

  uint16_t mqttPort = ControllerSettings->Port;

  mqtt_tls_last_errorstr.clear();
  mqtt_tls_last_error = 0;
  const TLS_types TLS_type = ControllerSettings->TLStype();

  if ((TLS_type != TLS_types::NoTLS) && (nullptr == mqtt_tls)) {
#  ifdef ESP32
  #   if MQTT_MAX_PACKET_SIZE > 2000
    mqtt_tls = new BearSSL::WiFiClientSecure_light(4096, 4096);
  #   else // if MQTT_MAX_PACKET_SIZE > 2000
    mqtt_tls = new BearSSL::WiFiClientSecure_light(2048, 2048);
  #   endif // if MQTT_MAX_PACKET_SIZE > 2000
#  else // ESP32 - ESP8266
    mqtt_tls = new BearSSL::WiFiClientSecure_light(1024, 1024);
#  endif // ifdef ESP32
    mqtt_rootCA.clear();

    if (mqtt_tls == nullptr) {
      mqtt_tls_last_errorstr = F("MQTT : Could not create TLS client, out of memory");
      addLog(LOG_LEVEL_ERROR, mqtt_tls_last_errorstr);
      return false;
    } else {
      mqtt_tls->setUtcTime_fcn(getUnixTime);
      mqtt_tls->setCfgTime_fcn(get_build_unixtime);
    }
  }

  switch (TLS_type) {
    case TLS_types::NoTLS:
    {
      // Ignoring the ACK from the server is probably set for a reason.
      // For example because the server does not give an acknowledgement.
      // This way, we always need the set amount of timeout to handle the request.
      // Thus we should not make the timeout dynamic here if set to ignore ack.
      const uint32_t timeout = ControllerSettings->MustCheckReply
      ? WiFiEventData.getSuggestedTimeout(Settings.Protocol[controller_idx], ControllerSettings->ClientTimeout)
      : ControllerSettings->ClientTimeout;

  #  ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

      // See: https://github.com/espressif/arduino-esp32/pull/6676
      mqtt.setTimeout((timeout + 500) / 1000); // in seconds!!!!
      Client *pClient = &mqtt;
      pClient->setTimeout(timeout);
  #  else // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS
      mqtt.setTimeout(timeout); // in msec as it should be!
  #  endif // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS
      MQTTclient.setClient(mqtt);
      MQTTclient.setKeepAlive(ControllerSettings->KeepAliveTime);
      MQTTclient.setSocketTimeout(timeout);
      break;
    }

    /*
       case TLS_types::TLS_PSK:
       {
       // if (mqtt_tls != nullptr)
       //  mqtt_tls->setPreSharedKey(const char *pskIdent, const char *psKey); // psKey in Hex
       break;
       }
     */
    case TLS_types::TLS_CA_CERT:
    {
      mqtt_rootCA.clear();

      /*
         // FIXME TD-er: Must convert rootCA from file to format accepted by bearSSL

         if (mqtt_rootCA.isEmpty() && (mqtt_tls != nullptr)) {
         LoadCertificate(ControllerSettings->getCertificateFilename(), mqtt_rootCA);

         if (mqtt_rootCA.isEmpty()) {
          // Fingerprint must be of some minimal length to continue.
          mqtt_tls_last_errorstr = F("MQTT : No TLS root CA");
          addLog(LOG_LEVEL_ERROR, mqtt_tls_last_errorstr);
          return false;
         }



         //mqtt_X509List.append(mqtt_rootCA.c_str());
         //        mqtt_tls->setTrustAnchors(&mqtt_X509List);
         }
       */
      if (mqtt_tls != nullptr) {
        mqtt_tls->setTrustAnchor(Tasmota_TA, Tasmota_TA_size);
      }
      break;
    }

    /*
       case TLS_types::TLS_CA_CLI_CERT:
       {
       //if (mqtt_tls != nullptr)
       //  mqtt_tls->setCertificate(const char *client_ca);
       break;
       }
     */
    case TLS_types::TLS_FINGERPRINT:
    {
      // Fingerprint is checked when making the connection.
      mqtt_rootCA.clear();
      mqtt_fingerprint.clear();
      LoadCertificate(ControllerSettings->getCertificateFilename(), mqtt_fingerprint, false);

      if (mqtt_fingerprint.length() < 32) {
        // Fingerprint must be of some minimal length to continue.
        mqtt_tls_last_errorstr = F("MQTT : Stored TLS fingerprint too small");
        addLog(LOG_LEVEL_ERROR, mqtt_tls_last_errorstr);
        return false;
      }

      if (mqtt_tls != nullptr) {
        mqtt_tls->setInsecure();
      }
      break;
    }
    case TLS_types::TLS_insecure:
    {
      mqtt_rootCA.clear();

      if (mqtt_tls != nullptr) {
        mqtt_tls->setInsecure();
      }
      break;
    }
  }

  if ((TLS_type != TLS_types::NoTLS) && (mqtt_tls != nullptr)) {
    // Certificate expiry not enabled in Mbed TLS.
    //    mqtt_tls->setX509Time(node_time.getUnixTime());
    // Ignoring the ACK from the server is probably set for a reason.
    // For example because the server does not give an acknowledgement.
    // This way, we always need the set amount of timeout to handle the request.
    // Thus we should not make the timeout dynamic here if set to ignore ack.
    const uint32_t timeout = ControllerSettings->MustCheckReply
      ? WiFiEventData.getSuggestedTimeout(Settings.Protocol[controller_idx], ControllerSettings->ClientTimeout)
      : ControllerSettings->ClientTimeout;

#  ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

    // See: https://github.com/espressif/arduino-esp32/pull/6676
    mqtt_tls->setTimeout((timeout + 500) / 1000); // in seconds!!!!
    Client *pClient = mqtt_tls;
    pClient->setTimeout(timeout);
#  else // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS
    mqtt_tls->setTimeout(timeout); // in msec as it should be!
#  endif // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

#  ifdef ESP8266
    mqtt_tls->setBufferSizes(1024, 1024);
    #  endif // ifdef ESP8266
    MQTTclient.setClient(*mqtt_tls);
    MQTTclient.setKeepAlive(ControllerSettings->KeepAliveTime);
    MQTTclient.setSocketTimeout(timeout);


    if (mqttPort == 1883) {
      mqttPort = 8883;
    }
  } else {
    if (mqttPort == 8883) {
      mqttPort = 1883;
    }
  }

# else // if FEATURE_MQTT_TLS

  // Ignoring the ACK from the server is probably set for a reason.
  // For example because the server does not give an acknowledgement.
  // This way, we always need the set amount of timeout to handle the request.
  // Thus we should not make the timeout dynamic here if set to ignore ack.
  const uint32_t timeout = ControllerSettings->MustCheckReply
    ? WiFiEventData.getSuggestedTimeout(Settings.Protocol[controller_idx], ControllerSettings->ClientTimeout)
    : ControllerSettings->ClientTimeout;

#  ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

  // See: https://github.com/espressif/arduino-esp32/pull/6676
  mqtt.setTimeout((timeout + 500) / 1000); // in seconds!!!!
  Client *pClient = &mqtt;
  pClient->setTimeout(timeout);
#  else // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS
  mqtt.setTimeout(timeout); // in msec as it should be!
#  endif // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

  MQTTclient.setClient(mqtt);
  MQTTclient.setKeepAlive(ControllerSettings->KeepAliveTime);
  MQTTclient.setSocketTimeout(timeout);
# endif // if FEATURE_MQTT_TLS

  if (ControllerSettings->UseDNS) {
    MQTTclient.setServer(ControllerSettings->getHost().c_str(), ControllerSettings->Port);
  } else {
    MQTTclient.setServer(ControllerSettings->getIP(), ControllerSettings->Port);
  }
  MQTTclient.setCallback(incoming_mqtt_callback);

  // MQTT needs a unique clientname to subscribe to broker
  const String clientid = getMQTTclientID(*ControllerSettings);

  const String LWTTopic             = getLWT_topic(*ControllerSettings);
  const String LWTMessageDisconnect = getLWT_messageDisconnect(*ControllerSettings);
  bool MQTTresult                   = false;
  const uint8_t willQos             = 0;
  const bool    willRetain          = ControllerSettings->mqtt_willRetain() && ControllerSettings->mqtt_sendLWT();
  const bool    cleanSession        = ControllerSettings->mqtt_cleanSession(); // As suggested here:

  if (MQTTclient_should_reconnect) {
    addLog(LOG_LEVEL_ERROR, F("MQTT : Intentional reconnect"));
  }

  const uint64_t statisticsTimerStart(getMicros64());

  // https://github.com/knolleary/pubsubclient/issues/458#issuecomment-493875150
  if (hasControllerCredentialsSet(controller_idx, *ControllerSettings)) {
    MQTTresult =
      MQTTclient.connect(clientid.c_str(),
                         getControllerUser(controller_idx, *ControllerSettings).c_str(),
                         getControllerPass(controller_idx, *ControllerSettings).c_str(),
                         ControllerSettings->mqtt_sendLWT() ? LWTTopic.c_str() : nullptr,
                         willQos,
                         willRetain,
                         ControllerSettings->mqtt_sendLWT() ? LWTMessageDisconnect.c_str() : nullptr,
                         cleanSession);
  } else {
    MQTTresult = MQTTclient.connect(clientid.c_str(),
                                    nullptr,
                                    nullptr,
                                    ControllerSettings->mqtt_sendLWT() ? LWTTopic.c_str() : nullptr,
                                    willQos,
                                    willRetain,
                                    ControllerSettings->mqtt_sendLWT() ? LWTMessageDisconnect.c_str() : nullptr,
                                    cleanSession);
  }
  delay(0);

  count_connection_results(
    MQTTresult,
    F("MQTT : Broker "),
    Settings.Protocol[controller_idx],
    statisticsTimerStart);

  # if FEATURE_MQTT_TLS

  if (mqtt_tls != nullptr)
  {
    #  ifdef ESP32
    mqtt_tls_last_error = mqtt_tls->getLastError();
    mqtt_tls->clearLastError();
    #  endif // ifdef ESP32
    // mqtt_tls_last_errorstr = buf;
  }
  #  ifdef ESP32

  // FIXME TD-er: There seems to be no verify function in BearSSL used on ESP8266
  if (TLS_type == TLS_types::TLS_FINGERPRINT)
  {
    // Check fingerprint
    if (MQTTresult) {
      const int newlinepos = mqtt_fingerprint.indexOf('\n');
      String    fp;
      String    dn;

      if (ControllerSettings->UseDNS) { dn = ControllerSettings->getHost(); }

      if (newlinepos == -1) {
        fp = mqtt_fingerprint;
      } else {
        fp = mqtt_fingerprint.substring(0, newlinepos);
        const int newlinepos2 = mqtt_fingerprint.indexOf('\n', newlinepos);

        if (newlinepos2 == -1) {
          dn = mqtt_fingerprint.substring(newlinepos + 1);
        }
        else {
          dn = mqtt_fingerprint.substring(newlinepos + 1, newlinepos2);
        }
        dn.trim();
      }

      // FIXME TD-er: Must implement fingerprint verification

      /*
         if (mqtt_tls != nullptr) {
         if (!mqtt_tls->verify(
              fp.c_str(),
              dn.isEmpty() ? nullptr : dn.c_str()))
         {
          mqtt_tls_last_errorstr += F("TLS Fingerprint does not match");
          addLog(LOG_LEVEL_INFO, mqtt_fingerprint);
          MQTTresult = false;
         }
         }
       */
    }
  }
  #  endif // ifdef ESP32

  # endif  // if FEATURE_MQTT_TLS

  if (!MQTTresult) {
    # if FEATURE_MQTT_TLS

    if ((mqtt_tls_last_error != 0) && loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("MQTT : TLS error code: ");
      log += mqtt_tls_last_error;
      log += ' ';
      log += mqtt_tls_last_errorstr;
      addLog(LOG_LEVEL_ERROR, log);
    }
    # endif // if FEATURE_MQTT_TLS

    MQTTclient.disconnect();
    # if FEATURE_MQTT_TLS

    if (mqtt_tls != nullptr) {
      mqtt_tls->stop();
    }
    # endif // if FEATURE_MQTT_TLS

    updateMQTTclient_connected();

    return false;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    addLogMove(LOG_LEVEL_INFO, concat(F("MQTT : Connected to broker with client ID: "), clientid));
  }

  # if FEATURE_MQTT_TLS
  #  ifdef ESP32

  // FIXME TD-er: Must get certificate info

  /*

     if ((mqtt_tls != nullptr) && loglevelActiveFor(LOG_LEVEL_INFO))
     {
     String log = F("MQTT : Peer certificate info: ");
     log += ControllerSettings->getHost();
     log += ' ';
     log += mqtt_tls->getPeerCertificateInfo();
     addLogMove(LOG_LEVEL_INFO, log);
     }
   */
  #  endif // ifdef ESP32
  # endif  // if FEATURE_MQTT_TLS

  String subscribeTo = ControllerSettings->Subscribe;

  parseSystemVariables(subscribeTo, false);
  MQTTclient.subscribe(subscribeTo.c_str());

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    addLogMove(LOG_LEVEL_INFO, concat(F("Subscribed to: "),  subscribeTo));
  }

  updateMQTTclient_connected();
  statusLED(true);
  mqtt_reconnect_count = 0;

  // call all installed controller to publish autodiscover data
  if (MQTTclient_should_reconnect) { CPluginCall(CPlugin::Function::CPLUGIN_GOT_CONNECTED, 0); }
  MQTTclient_should_reconnect = false;

  if (ControllerSettings->mqtt_sendLWT()) {
    String LWTMessageConnect = getLWT_messageConnect(*ControllerSettings);

    if (!MQTTclient.publish(LWTTopic.c_str(), LWTMessageConnect.c_str(), willRetain)) {
      MQTTclient_must_send_LWT_connected = true;
    }
  }

  return true;
}

String getMQTTclientID(const ControllerSettingsStruct& ControllerSettings) {
  String clientid = ControllerSettings.ClientID;

  if (clientid.isEmpty()) {
    // Try to generate some default
    clientid = F(CONTROLLER_DEFAULT_CLIENTID);
  }
  parseSystemVariables(clientid, false);
  clientid.replace(' ', '_'); // Make sure no spaces are present in the client ID

  if ((WiFiEventData.wifi_reconnects >= 1) && ControllerSettings.mqtt_uniqueMQTTclientIdReconnect()) {
    // Work-around for 'lost connections' to the MQTT broker.
    // If the broker thinks the connection is still alive, a reconnect from the
    // client will be refused.
    // To overcome this issue, append the number of reconnects to the client ID to
    // make it different from the previous one.
    clientid += '_';
    clientid += WiFiEventData.wifi_reconnects;
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

  if (getProtocolStruct(ProtocolIndex).usesMQTT)
  {
    bool   mqtt_sendLWT = false;
    String LWTTopic, LWTMessageConnect;
    bool   willRetain = false;
    {
      MakeControllerSettings(ControllerSettings); // -V522

      if (!AllocatedControllerSettings()) {
        addLog(LOG_LEVEL_ERROR, F("MQTT : Cannot check, out of RAM"));
        return false;
      }

      LoadControllerSettings(controller_idx, *ControllerSettings);

      // FIXME TD-er: Is this still needed?

      /*
       #ifdef USES_ESPEASY_NOW
         if (!MQTTclient.connected()) {
         if (ControllerSettings->enableESPEasyNowFallback()) {
          return true;
         }
         }
       #endif
       */

      if (!ControllerSettings->isSet()) {
        return true;
      }

      if (ControllerSettings->mqtt_sendLWT()) {
        mqtt_sendLWT      = true;
        LWTTopic          = getLWT_topic(*ControllerSettings);
        LWTMessageConnect = getLWT_messageConnect(*ControllerSettings);
        willRetain        = ControllerSettings->mqtt_willRetain();
      }
    }

    if (MQTTclient_should_reconnect || !MQTTclient.connected())
    {
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

    if (LWTTopic.isEmpty())
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

    if (LWTMessageConnect.isEmpty()) {
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

    if (LWTMessageDisconnect.isEmpty()) {
      LWTMessageDisconnect = F(DEFAULT_MQTT_LWT_DISCONNECT_MESSAGE);
    }
    parseSystemVariables(LWTMessageDisconnect, false);
  }
  return LWTMessageDisconnect;
}

#endif // if FEATURE_MQTT

/*********************************************************************************************\
* Send status info to request source
\*********************************************************************************************/
void SendStatusOnlyIfNeeded(struct EventStruct *event, bool param1, uint32_t key, const String& param2, int16_t param3) {
  if (SourceNeedsStatusUpdate(event->Source)) {
    SendStatus(event, getPinStateJSON(param1, key, param2, param3));
    printToWeb = false; // SP: 2020-06-12: to avoid to add more info to a JSON structure
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

void SendStatus(struct EventStruct *event, const __FlashStringHelper *status)
{
  SendStatus(event, String(status));
}

void SendStatus(struct EventStruct *event, const String& status)
{
  if (status.isEmpty()) { return; }

  switch (event->Source)
  {
    case EventValueSource::Enum::VALUE_SOURCE_HTTP:
    case EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND:

      if (printToWeb) {
        printWebString += status;
      }
      break;
#if FEATURE_MQTT
    case EventValueSource::Enum::VALUE_SOURCE_MQTT:
      MQTTStatus(event, status);
      break;
#endif // if FEATURE_MQTT
    case EventValueSource::Enum::VALUE_SOURCE_SERIAL:
      serialPrintln(status);
      break;

    default:
      break;
  }
}

#if FEATURE_MQTT
controllerIndex_t firstEnabledMQTT_ControllerIndex() {
  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; ++i) {
    protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(i);

    if (validProtocolIndex(ProtocolIndex)) {
      if (getProtocolStruct(ProtocolIndex).usesMQTT && Settings.ControllerEnabled[i]) {
        return i;
      }
    }
  }
  return INVALID_CONTROLLER_INDEX;
}

bool MQTT_queueFull(controllerIndex_t controller_idx) {
  if (MQTTDelayHandler == nullptr) {
    return true;
  }

  if (MQTTDelayHandler->queueFull(controller_idx)) {
    // The queue is full, try to make some room first.
    processMQTTdelayQueue();
    return MQTTDelayHandler->queueFull(controller_idx);
  }
  return false;
}

bool MQTTpublish(controllerIndex_t controller_idx,
                 taskIndex_t       taskIndex,
                 const char       *topic,
                 const char       *payload,
                 bool              retained,
                 bool              callbackTask)
{
  if (MQTTDelayHandler == nullptr) {
    return false;
  }

  if (MQTT_queueFull(controller_idx)) {
    return false;
  }
  const bool success =
    MQTTDelayHandler->addToQueue(std::unique_ptr<MQTT_queue_element>(new (std::nothrow) MQTT_queue_element(controller_idx, taskIndex, topic,
                                                                                                           payload, retained,
                                                                                                           callbackTask)));

  scheduleNextMQTTdelayQueue();
  return success;
}

bool MQTTpublish(controllerIndex_t controller_idx,
                 taskIndex_t       taskIndex,
                 String         && topic,
                 String         && payload,
                 bool              retained,
                 bool              callbackTask) {
  if (MQTTDelayHandler == nullptr) {
    return false;
  }

  if (MQTT_queueFull(controller_idx)) {
    return false;
  }

  const bool success =
    MQTTDelayHandler->addToQueue(std::unique_ptr<MQTT_queue_element>(new (std::nothrow) MQTT_queue_element(controller_idx, taskIndex,
                                                                                                           std::move(topic),
                                                                                                           std::move(payload), retained,
                                                                                                           callbackTask)));

  scheduleNextMQTTdelayQueue();
  return success;
}

/*********************************************************************************************\
* Send status info back to channel where request came from
\*********************************************************************************************/
void MQTTStatus(struct EventStruct *event, const String& status)
{
  controllerIndex_t enabledMqttController = firstEnabledMQTT_ControllerIndex();

  if (validControllerIndex(enabledMqttController)) {
    controllerIndex_t DomoticzMQTT_controllerIndex = findFirstEnabledControllerWithId(2);

    if (DomoticzMQTT_controllerIndex == enabledMqttController) {
      // Do not send MQTT status updates to Domoticz
      return;
    }
    String pubname;
    bool   mqtt_retainFlag;
    {
      // Place the ControllerSettings in a scope to free the memory as soon as we got all relevant information.
      MakeControllerSettings(ControllerSettings); // -V522

      if (!AllocatedControllerSettings()) {
        addLog(LOG_LEVEL_ERROR, F("MQTT : Cannot send status, out of RAM"));
        return;
      }

      LoadControllerSettings(enabledMqttController, *ControllerSettings);
      pubname         = ControllerSettings->Publish;
      mqtt_retainFlag = ControllerSettings->mqtt_retainFlag();
    }

    // FIXME TD-er: Why check for "/#" suffix on a publish topic?
    // It makes no sense to have a subscribe wildcard on a publish topic.
    pubname.replace(F("/#"), F("/status"));

    parseSingleControllerVariable(pubname, event, 0, false);
    parseControllerVariables(pubname, event, false);


    if (!pubname.endsWith(F("/status"))) {
      pubname += F("/status");
    }

    MQTTpublish(enabledMqttController, event->TaskIndex, pubname.c_str(), status.c_str(), mqtt_retainFlag);
  }
}

# if FEATURE_MQTT_TLS
bool GetTLSfingerprint(String& fp)
{
  #  ifdef ESP32

  if (MQTTclient_connected && (mqtt_tls != nullptr)) {
    const uint8_t *recv_fingerprint = mqtt_tls->getRecvPubKeyFingerprint();

    if (recv_fingerprint != nullptr) {
      fp.reserve(64);

      for (size_t i = 0; i < 21; ++i) {
        const String tmp(recv_fingerprint[i], HEX);

        switch (tmp.length()) {
          case 0:
            fp += '0';

          // fall through
          case 1:
            fp += '0';
            break;
        }
        fp += tmp;
      }
      fp.toLowerCase();
      return true;
    }
  }
  #  endif // ifdef ESP32
  return false;
}

bool GetTLS_Certificate(String& cert, bool caRoot)
{
  #  ifdef ESP32

  // FIXME TD-er: Implement retrieval of certificate

  /*

     if (MQTTclient_connected && (mqtt_tls != nullptr)) {
     String subject;

     if (mqtt_tls->getPeerCertificate(cert, subject, caRoot) == 0) {
      return true;
     }
     }
   */
  #  endif // ifdef ESP32
  return false;
}

# endif // if FEATURE_MQTT_TLS

#endif  // if FEATURE_MQTT


/*********************************************************************************************\
* send specific sensor task data, effectively calling PluginCall(PLUGIN_READ...)
\*********************************************************************************************/
void SensorSendTask(struct EventStruct *event, unsigned long timestampUnixTime)
{
  SensorSendTask(event, timestampUnixTime, millis());
}

void SensorSendTask(struct EventStruct *event, unsigned long timestampUnixTime, unsigned long lasttimer)
{
  if (!validTaskIndex(event->TaskIndex)) { return; }

  // FIXME TD-er: Should a 'disabled' task be rescheduled?
  // If not, then it should be rescheduled after the check to see if it is enabled.
  Scheduler.reschedule_task_device_timer(event->TaskIndex, lasttimer);

  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SensorSendTask"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  if (Settings.TaskDeviceEnabled[event->TaskIndex])
  {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

    if (!validDeviceIndex(DeviceIndex)) { return; }

    struct EventStruct TempEvent(event->TaskIndex);
    TempEvent.Source        = event->Source;
    TempEvent.timestamp_sec = timestampUnixTime;
    checkDeviceVTypeForTask(&TempEvent);

    String dummy;

    if (PluginCall(PLUGIN_READ, &TempEvent, dummy)) {
      sendData(&TempEvent);
    }
  }
}
