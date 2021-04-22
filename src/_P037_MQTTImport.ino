#include "_Plugin_Helper.h"
#ifdef USES_P037
//#######################################################################################################
//#################################### Plugin 037: MQTT Import ##########################################
//#######################################################################################################


// Original plugin created by Namirda

// This task reads data from the MQTT Import input stream and saves the value

#include "src/Globals/EventQueue.h"
#include "src/Globals/MQTT.h"
#include "src/Globals/CPlugins.h"
#include "src/Globals/Plugins.h"
#include "src/Helpers/ESPEasy_Storage.h"
#include "src/Helpers/Misc.h"
#include "src/Helpers/StringParser.h"

#define PLUGIN_037
#define PLUGIN_ID_037         37
#define PLUGIN_NAME_037       "Generic - MQTT Import"

#define PLUGIN_VALUENAME1_037 "Value1"
#define PLUGIN_VALUENAME2_037 "Value2"
#define PLUGIN_VALUENAME3_037 "Value3"
#define PLUGIN_VALUENAME4_037 "Value4"


boolean Plugin_037(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_037;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SINGLE;     // This means it has a single pin
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true; // Need this in order to get the decimals option
        Device[deviceCount].ValueCount = VARS_PER_TASK;
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
        char deviceTemplate[VARS_PER_TASK][41];		// variable for saving the subscription topics
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
        {
        	addFormTextBox(String(F("MQTT Topic ")) + (varNr + 1), String(F("p037_template")) +
        			(varNr + 1), deviceTemplate[varNr], 40);
        }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String error;
        char deviceTemplate[VARS_PER_TASK][41];		// variable for saving the subscription topics
        for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
        {
          String argName = F("p037_template");
          argName += varNr + 1;
          if (!safe_strncpy(deviceTemplate[varNr], web_server.arg(argName).c_str(), sizeof(deviceTemplate[varNr]))) {
            error += getCustomTaskSettingsError(varNr);
          }
        }
        if (error.length() > 0) {
          addHtmlError(error);
        }

        SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        success = false;
        //    When we edit the subscription data from the webserver, the plugin is called again with init.
        //    In order to resubscribe we have to disconnect and reconnect in order to get rid of any obsolete subscriptions
        if (MQTTclient_connected) {
          //		Subscribe to ALL the topics from ALL instance of this import module
          MQTTSubscribe_037(event);
          success = true;
        }
      }
      break;

    case PLUGIN_READ:
      {
        // This routine does not output any data and so we do not need to respond to regular read requests

        success = false;
        break;
      }

    case PLUGIN_MQTT_CONNECTION_STATE:
      {
        const bool currentConnectedState = event->Par1 == 1;
        if (P037_MQTTImport_connected != currentConnectedState) {
          P037_MQTTImport_connected = currentConnectedState;
          if (Settings.UseRules)
          {
            eventQueue.add(currentConnectedState ? F("MQTTimport#Connected") : F("MQTTimport#Disconnected"));
          }
        }

        if (currentConnectedState) {
          success = MQTTSubscribe_037(event);
        }
        break;
      }

    case PLUGIN_MQTT_IMPORT:
      {
        // Get the payload and check it out
        //   Topic:   event->String1;
        //   Payload: event->String2;

        // Get the Topic and see if it matches any of the subscriptions
        char deviceTemplate[VARS_PER_TASK][41];		// variable for saving the subscription topics

        LoadTaskSettings(event->TaskIndex);
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        for (byte x = 0; x < VARS_PER_TASK; x++)
        {
          String subscriptionTopic = deviceTemplate[x];
          subscriptionTopic.trim();
          if (subscriptionTopic.length() == 0) continue;							// skip blank subscriptions

          // Now check if the incoming topic matches one of our subscriptions
          parseSystemVariables(subscriptionTopic, false);
          if (MQTTCheckSubscription_037(event->String1, subscriptionTopic))
          {
            // FIXME TD-er: It may be useful to generate events with string values.
            float floatPayload;
            if (!string2float(event->String2, floatPayload)) {
              String log = F("IMPT : Bad Import MQTT Command ");
              log += event->String1;
              addLog(LOG_LEVEL_ERROR, log);
              log = F("ERR  : Illegal Payload ");
              log += event->String2;
              log += ' ';
              log += getTaskDeviceName(event->TaskIndex);
              addLog(LOG_LEVEL_INFO, log);
              success = false;
              break;
            }

            UserVar[event->BaseVarIndex + x] = floatPayload;							// Save the new value

            // Log the event
            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              String log = F("IMPT : [");
              log += getTaskDeviceName(event->TaskIndex);
              log += F("#");
              log += ExtraTaskSettings.TaskDeviceValueNames[x];
              log += F("] : ");
              log += floatPayload;
              addLog(LOG_LEVEL_INFO, log);
            }

            // Generate event for rules processing - proposed by TridentTD

            if (Settings.UseRules)
            {
              String RuleEvent;
              RuleEvent += getTaskDeviceName(event->TaskIndex);
              RuleEvent += '#';
              RuleEvent += ExtraTaskSettings.TaskDeviceValueNames[x];
              RuleEvent += '=';
              RuleEvent += floatPayload;
              eventQueue.addMove(std::move(RuleEvent));
            }

            success = true;
          }
        }

        break;

      }
  }

  return success;
}
bool MQTTSubscribe_037(struct EventStruct *event)
{
  // We must subscribe to the topics.
  char deviceTemplate[VARS_PER_TASK][41];		// variable for saving the subscription topics
  LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

  // Now loop over all import variables and subscribe to those that are not blank
  for (byte x = 0; x < VARS_PER_TASK; x++)
  {
    String subscribeTo = deviceTemplate[x];

    if (subscribeTo.length() > 0)
    {
      parseSystemVariables(subscribeTo, false);
      if (MQTTclient.subscribe(subscribeTo.c_str()))
      {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("IMPT : [");
          LoadTaskSettings(event->TaskIndex);
          log += getTaskDeviceName(event->TaskIndex);
          log += F("#");
          log += ExtraTaskSettings.TaskDeviceValueNames[x];
          log += F("] subscribed to ");
          log += subscribeTo;
          addLog(LOG_LEVEL_INFO, log);
        }
      }
      else
      {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log = F("IMPT : Error subscribing to ");
          log += subscribeTo;
          addLog(LOG_LEVEL_ERROR, log);
          return false;
        }
      }
    }
  }
  return true;
}

//
// Check to see if Topic matches the MQTT subscription
//
bool MQTTCheckSubscription_037(const String& Topic, const String& Subscription) {
  if (Topic.length() == 0 || Subscription.length() == 0)  {
    return false;
  }

  String tmpTopic = Topic;
  String tmpSub = Subscription;

  tmpTopic.trim();
  tmpSub.trim();

  // Get rid of leading '/'
  if (tmpTopic[0] == '/') { tmpTopic = tmpTopic.substring(1); }
  if (tmpSub[0] == '/') { tmpSub = tmpSub.substring(1); }

  // Add trailing / if required

  int lenTopic = tmpTopic.length();
  if (tmpTopic.substring(lenTopic - 1, lenTopic) != "/")tmpTopic += '/';

  int lenSub = tmpSub.length();
  if (tmpSub.substring(lenSub - 1, lenSub) != "/")tmpSub += '/';

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
