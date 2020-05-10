#ifdef USES_P098

// #######################################################################################################
// #################################### Plugin 098: ESPEasy-Now Receiver #################################
// #######################################################################################################

#include "_Plugin_Helper.h"
#include "src/Helpers/ESPEasy_now.h"


#define PLUGIN_098
#define PLUGIN_ID_098         98
#define PLUGIN_NAME_098       "Generic - ESPEasy-Now Receiver"
#define PLUGIN_VALUENAME1_098 "Value"





void p098_onReceive(const uint8_t mac[6], const uint8_t* buf, size_t count, void* cbarg) {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("ESPEasyNow: Message from ");
    log += formatMAC(mac);
    addLog(LOG_LEVEL_INFO, log);
  }

  #ifdef USES_MQTT

  // FIXME TD-er: Quick hack to just echo all data to the first enabled MQTT controller

  controllerIndex_t controllerIndex = firstEnabledMQTT_ControllerIndex();
  if (!validControllerIndex(controllerIndex)) {
    return;
  }

  static size_t topic_length = 0;
  static size_t payload_length = 0;
  String topic, payload;
  topic.reserve(topic_length);
  payload.reserve(payload_length);
  bool topic_done = false;
  for (size_t i = 0; i < count; ++i) {
    if (!topic_done) {
      if (buf[i] != 0) {
        topic += static_cast<char>(buf[i]);
      } else {
        // Update estimates for next call.
        if (topic_length < i) {
          topic_length = i;
        }
        if (payload_length < (count - i)) {
          payload_length = (count - i);
        }
        topic_done = true;
      }
    } else {
      if (buf[i] != 0) {
        payload += static_cast<char>(buf[i]);
      }
    }
  }

  MakeControllerSettings(ControllerSettings);
  LoadControllerSettings(controllerIndex, ControllerSettings);
  MQTTpublish(controllerIndex, topic.c_str(), payload.c_str(), ControllerSettings.mqtt_retainFlag());

  #endif
}

boolean Plugin_098(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_098;
      Device[deviceCount].Type               = DEVICE_TYPE_DUMMY;
      Device[deviceCount].VType              = SENSOR_TYPE_STRING; // FIXME TD-er: Must make this the same as the sender.
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].DecimalsOnly       = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].GlobalSyncOption   = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_098);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_098));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // Do not set the sensor type, or else it will be set for all instances of the Dummy plugin.
      // sensorTypeHelper_setSensorType(event, 0);

      WifiEspNow.onReceive(p098_onReceive, nullptr);
      for (byte peer = 0; peer < ESPEASY_NOW_PEER_MAX; ++peer) {
        if (SecuritySettings.peerMacSet(peer)) {
          WifiEspNow.addPeer(SecuritySettings.EspEasyNowPeerMAC[peer]);
        }
      }

      plugin_EspEasy_now_enabled = true;
      success = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      plugin_EspEasy_now_enabled = false;
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      event->sensorType = PCONFIG(0);
      success           = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      // FIXME TD-er: Create commands for ESPEasy_now receiver
      String command = parseString(string, 1);
      if (command == F("espeasynow")) {
        String subcommand = parseString(string, 2);
        if (subcommand == F("")) {

        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P098
