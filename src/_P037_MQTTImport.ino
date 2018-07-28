#ifdef USES_P037
//#######################################################################################################
//#################################### Plugin 037: MQTT Import ##########################################
//#######################################################################################################

// Original plugin created by Namirda

// This task reads data from the MQTT Import input stream and saves the value

#define PLUGIN_037
#define PLUGIN_ID_037         37
#define PLUGIN_NAME_037       "Generic - MQTT Import"

#define PLUGIN_VALUENAME1_037 "Value1"
#define PLUGIN_VALUENAME2_037 "Value2"
#define PLUGIN_VALUENAME3_037 "Value3"
#define PLUGIN_VALUENAME4_037 "Value4"

#define PLUGIN_IMPORT 37		// This is a 'private' function used only by this import module

// Declare a Wifi client for this plugin only

// TODO TD-er: These must be kept in some vector to allow multiple instances of MQTT import.
WiFiClient espclient_037;
PubSubClient *MQTTclient_037 = NULL;
bool MQTTclient_037_connected = false;

void Plugin_037_update_connect_status() {
  bool connected = false;
  if (MQTTclient_037 != NULL) {
    connected = MQTTclient_037->connected();
  }
  if (MQTTclient_037_connected != connected) {
    MQTTclient_037_connected = !MQTTclient_037_connected;
    if (Settings.UseRules) {
      String event = connected ? F("MQTTimport#Connected") : F("MQTTimport#Disconnected");
      rulesProcessing(event);
    }
    if (!connected) {
      // workaround see: https://github.com/esp8266/Arduino/issues/4497#issuecomment-373023864
      espclient_037 = WiFiClient();
      addLog(LOG_LEVEL_ERROR, F("IMPT : MQTT 037 Connection lost"));
    }
  }
}

boolean Plugin_037(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  char deviceTemplate[4][41];		// variable for saving the subscription topics
  //
  // Generate the MQTT import client name from the system name and a suffix
  //
  String tmpClientName = F("%sysname%-Import");
  String ClientName = parseTemplate(tmpClientName, 20);

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_037;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;     // This means it has a single pin
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true; // Need this in order to get the decimals option
        Device[deviceCount].ValueCount = 4;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_037);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_037));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_037));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_037));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_037));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {

        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        for (byte varNr = 0; varNr < 4; varNr++)
        {
        	addFormTextBox(String(F("MQTT Topic ")) + (varNr + 1), String(F("Plugin_037_template")) +
        			(varNr + 1), deviceTemplate[varNr], 40);
        }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String argName;

        for (byte varNr = 0; varNr < 4; varNr++)
        {
          argName = F("Plugin_037_template");
          argName += varNr + 1;
          strncpy(deviceTemplate[varNr], WebServer.arg(argName).c_str(), sizeof(deviceTemplate[varNr]));
        }

        SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (!MQTTclient_037) {
          MQTTclient_037 = new PubSubClient(espclient_037);
        }

        //    When we edit the subscription data from the webserver, the plugin is called again with init.
        //    In order to resubscribe we have to disconnect and reconnect in order to get rid of any obsolete subscriptions

        MQTTclient_037->disconnect();

        if (MQTTConnect_037(ClientName))
        {
          //		Subscribe to ALL the topics from ALL instance of this import module
          MQTTSubscribe_037();
          success = true;
        }
        else
        {
          success = false;
        }
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (!MQTTclient_037->loop()) {		// Listen out for callbacks
          Plugin_037_update_connect_status();
        }
        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        //  Here we check that the MQTT client is alive.

        if (!MQTTclient_037->connected() || MQTTclient_should_reconnect) {
          if (MQTTclient_should_reconnect) {
            addLog(LOG_LEVEL_ERROR, F("IMPT : MQTT 037 Intentional reconnect"));
          }

          MQTTclient_037->disconnect();
          Plugin_037_update_connect_status();
          delay(250);

          if (! MQTTConnect_037(ClientName)) {
            success = false;
            break;
          }

          MQTTSubscribe_037();
        }

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // This routine does not output any data and so we do not need to respond to regular read requests

        success = false;
        break;
      }

    case PLUGIN_IMPORT:
      {
        // This is a private option only used by the MQTT 037 callback function

        //      Get the payload and check it out
        LoadTaskSettings(event->TaskIndex);

        String Payload = event->String2;
        float floatPayload;
        if (!string2float(Payload, floatPayload)) {
          String log = F("IMPT : Bad Import MQTT Command ");
          log += event->String1;
          addLog(LOG_LEVEL_ERROR, log);
          log = F("ERR  : Illegal Payload ");
          log += Payload;
          log += "  ";
          log += getTaskDeviceName(event->TaskIndex);
          addLog(LOG_LEVEL_INFO, log);
          success = false;
          break;
        }

        //      Get the Topic and see if it matches any of the subscriptions

        String Topic = event->String1;

        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        for (byte x = 0; x < 4; x++)
        {
          String subscriptionTopic = deviceTemplate[x];
          subscriptionTopic.trim();
          if (subscriptionTopic.length() == 0) continue;							// skip blank subscriptions

          // Now check if the incoming topic matches one of our subscriptions
          parseSystemVariables(subscriptionTopic, false);
          if (MQTTCheckSubscription_037(Topic, subscriptionTopic))
          {
            UserVar[event->BaseVarIndex + x] = floatPayload;							// Save the new value

            // Log the event

            String log = F("IMPT : [");
            log += getTaskDeviceName(event->TaskIndex);
            log += F("#");
            log += ExtraTaskSettings.TaskDeviceValueNames[x];
            log += F("] : ");
            log += floatPayload;
            addLog(LOG_LEVEL_INFO, log);

            // Generate event for rules processing - proposed by TridentTD

            if (Settings.UseRules)
            {
              String RuleEvent = F("");
              RuleEvent += getTaskDeviceName(event->TaskIndex);
              RuleEvent += F("#");
              RuleEvent += ExtraTaskSettings.TaskDeviceValueNames[x];
              RuleEvent += F("=");
              RuleEvent += floatPayload;
              rulesProcessing(RuleEvent);
            }

            success = true;
          }
        }

        break;

      }
  }

  return success;
}
boolean MQTTSubscribe_037()
{

  // Subscribe to the topics requested by ALL calls to this plugin.
  // We do this because if the connection to the broker is lost, we want to resubscribe for all instances.

  char deviceTemplate[4][41];

  //	Loop over all tasks looking for a 037 instance

  for (byte y = 0; y < TASKS_MAX; y++)
  {
    if (Settings.TaskDeviceNumber[y] == PLUGIN_ID_037)
    {
      LoadCustomTaskSettings(y, (byte*)&deviceTemplate, sizeof(deviceTemplate));

      // Now loop over all import variables and subscribe to those that are not blank

      for (byte x = 0; x < 4; x++)
      {
        String subscribeTo = deviceTemplate[x];

        if (subscribeTo.length() > 0)
        {
          parseSystemVariables(subscribeTo, false);
          if (MQTTclient_037->subscribe(subscribeTo.c_str()))
          {
            String log = F("IMPT : [");
            LoadTaskSettings(y);
            log += getTaskDeviceName(y);
            log += F("#");
            log += ExtraTaskSettings.TaskDeviceValueNames[x];
            log += F("] subscribed to ");
            log += subscribeTo;
            addLog(LOG_LEVEL_INFO, log);
          }
          else
          {
            String log = F("IMPT : Error subscribing to ");
            log += subscribeTo;
            addLog(LOG_LEVEL_ERROR, log);
            return false;
          }

        }
      }
    }
  }
  return true;
}
//
// handle MQTT messages
//
void mqttcallback_037(char* c_topic, byte* b_payload, unsigned int length)
{
  // Here we have incomng MQTT messages from the mqtt import module
  String topic = c_topic;

  char cpayload[256];
  strncpy(cpayload, (char*)b_payload, length);
  cpayload[length] = 0;
  String payload = cpayload;		// convert byte to char string
  payload.trim();

  byte DeviceIndex = getDeviceIndex(PLUGIN_ID_037);   // This is the device index of 037 modules -there should be one!

  // We generate a temp event structure to pass to the plugins

  struct EventStruct TempEvent;

  TempEvent.String1 = topic;                            // This is the topic of the message
  TempEvent.String2 = payload;                          // This is the payload

  //  Here we loop over all tasks and call each 037 plugin with function PLUGIN_IMPORT

  for (byte y = 0; y < TASKS_MAX; y++)
  {
    if (Settings.TaskDeviceNumber[y] == PLUGIN_ID_037)                // if we have found a 037 device, then give it something to think about!
    {
      TempEvent.TaskIndex = y;
      TempEvent.BaseVarIndex = y * VARS_PER_TASK;           // This is the index in Uservar where values for this task are stored
      Plugin_ptr[DeviceIndex](PLUGIN_IMPORT, &TempEvent, payload);
    }
  }
}

//
// Make a new client connection to the mqtt broker.
// For some reason this seems to failduring the call in INIT- however it succeeds later during recovery
// It would be nice to understand this....

boolean MQTTConnect_037(String clientid)
{
  boolean result = false;
  // @ToDo TD-er: Plugin allows for more than one MQTT controller, but we're now using only the first enabled one.
  int enabledMqttController = firstEnabledMQTTController();
  if (enabledMqttController < 0) {
    // No enabled MQTT controller
    return false;
  }
  // Do nothing if already connected
  if (MQTTclient_037->connected()) return true;

  // define stuff for the client - this could also be done in the intial declaration of MQTTclient_037
  if (!WiFiConnected(100)) {
    Plugin_037_update_connect_status();
    return false; // Not connected, so no use in wasting time to connect to a host.
  }
  ControllerSettingsStruct ControllerSettings;
  LoadControllerSettings(enabledMqttController, (byte*)&ControllerSettings, sizeof(ControllerSettings));
  if (ControllerSettings.UseDNS) {
    MQTTclient_037->setServer(ControllerSettings.getHost().c_str(), ControllerSettings.Port);
  } else {
    MQTTclient_037->setServer(ControllerSettings.getIP(), ControllerSettings.Port);
  }
  MQTTclient_037->setCallback(mqttcallback_037);

  //  Try three times for a connection

  for (byte x = 1; x < 4; x++)
  {
    String log = "";

    if ((SecuritySettings.ControllerUser[enabledMqttController][0] != 0) && (SecuritySettings.ControllerPassword[enabledMqttController][0] != 0))
      result = MQTTclient_037->connect(clientid.c_str(), SecuritySettings.ControllerUser[enabledMqttController], SecuritySettings.ControllerPassword[enabledMqttController]);
    else
      result = MQTTclient_037->connect(clientid.c_str());


    if (result)
    {
      log = F("IMPT : Connected to MQTT broker with Client ID=");
      log += clientid;
      addLog(LOG_LEVEL_INFO, log);

      break; // end loop if succesfull
    }
    else
    {
      log = F("IMPT : Failed to connect to MQTT broker - attempt ");
      log += x;
      addLog(LOG_LEVEL_ERROR, log);
    }

    delay(500);
  }
  Plugin_037_update_connect_status();
  return MQTTclient_037->connected();
}

//
// Check to see if Topic matches the MQTT subscription
//
boolean MQTTCheckSubscription_037(String Topic, String Subscription) {

  String tmpTopic = Topic;
  String tmpSub = Subscription;

  tmpTopic.trim();
  tmpSub.trim();

  // Get rid of any initial /

  if (tmpTopic.substring(0, 1) == "/")tmpTopic = tmpTopic.substring(1);
  if (tmpSub.substring(0, 1) == "/")tmpSub = tmpSub.substring(1);

  // Add trailing / if required

  int lenTopic = tmpTopic.length();
  if (tmpTopic.substring(lenTopic - 1, lenTopic) != "/")tmpTopic += F("/");

  int lenSub = tmpSub.length();
  if (tmpSub.substring(lenSub - 1, lenSub) != "/")tmpSub += F("/");

  // Now get first part

  int SlashTopic;
  int SlashSub;
  int count = 0;

  String pTopic;
  String pSub;

  while (count < 10) {

    //  Get locations of the first /

    SlashTopic = tmpTopic.indexOf('/');
    SlashSub = tmpSub.indexOf('/');

    //  If no slashes found then match is OK
    //  If only one slash found then not OK

    if ((SlashTopic == -1) && (SlashSub == -1)) return true;
    if ((SlashTopic == -1) && (SlashSub != -1)) return false;
    if ((SlashTopic != -1) && (SlashSub == -1)) return false;

    //  Get the values for the current subtopic

    pTopic = tmpTopic.substring(0, SlashTopic);
    pSub = tmpSub.substring(0, SlashSub);

    //  And strip the subtopic from the topic

    tmpTopic = tmpTopic.substring(SlashTopic + 1);
    tmpSub = tmpSub.substring(SlashSub + 1);

    //  If the subtopics match then OK - otherwise fail
    if (pSub == "#")  return true;
    if ((pTopic != pSub) && (pSub != "+"))return false;

    count = count + 1;
  }
  return false;
}
#endif // USES_P037
