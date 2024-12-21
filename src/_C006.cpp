#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C006

// #######################################################################################################
// ########################### Controller Plugin 006: PiDome MQTT ########################################
// #######################################################################################################

# include "src/Commands/ExecuteCommand.h"
# include "src/ESPEasyCore/Controller.h"
# include "src/Globals/Settings.h"
# include "src/Helpers/Network.h"
# include "src/Helpers/PeriodicalActions.h"
# include "_Plugin_Helper.h"

# define CPLUGIN_006
# define CPLUGIN_ID_006         6
# define CPLUGIN_NAME_006       "PiDome MQTT"

String CPlugin_006_pubname;
bool   CPlugin_006_mqtt_retainFlag = false;


bool CPlugin_006(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      ProtocolStruct& proto = getProtocolStruct(event->idx); //      = CPLUGIN_ID_006;
      proto.usesMQTT     = true;
      proto.usesTemplate = true;
      proto.usesAccount  = false;
      proto.usesPassword = false;
      proto.usesExtCreds = true;
      proto.defaultPort  = 1883;
      proto.usesID       = false;
      #if FEATURE_MQTT_TLS
      proto.usesTLS      = true;
      #endif
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_006);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_mqtt_delay_queue(event->ControllerIndex, CPlugin_006_pubname, CPlugin_006_mqtt_retainFlag);
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      exit_mqtt_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:
    {
      event->String1 = F("/Home/#");
      event->String2 = F("/hooks/devices/%id%/SensorData/%valname%");
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:
    {
      // topic structure /Home/Floor/Location/device/<systemname>/gpio/16
      // Split topic into array
      String tmpTopic = event->String1.substring(1);
      String topicSplit[10];
      int    SlashIndex = tmpTopic.indexOf('/');
      uint8_t   count      = 0;

      while (SlashIndex > 0 && count < 10 - 1)
      {
        topicSplit[count] = tmpTopic.substring(0, SlashIndex);
        tmpTopic          = tmpTopic.substring(SlashIndex + 1);
        SlashIndex        = tmpTopic.indexOf('/');
        count++;
      }
      topicSplit[count] = tmpTopic;

      String name = topicSplit[4];

      if (name.equals(Settings.getName()))
      {
        String cmd = topicSplit[5];
        cmd += ',';
        cmd += topicSplit[6].toInt(); // Par1
        cmd += ',';

        if ((event->String2.equalsIgnoreCase(F("false"))) || 
            (event->String2.equalsIgnoreCase(F("true"))))
        {
          cmd += (event->String2.equalsIgnoreCase(F("true"))) ? '1' : '0'; // Par2
        }
        else
        {
          cmd += event->String2; // Par2
        }
        ExecuteCommand_all({EventValueSource::Enum::VALUE_SOURCE_MQTT, std::move(cmd)}, true);
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      if (MQTT_queueFull(event->ControllerIndex)) {
        break;
      }

      String pubname              = CPlugin_006_pubname;
      const bool contains_valname = pubname.indexOf(F("%valname%")) != -1;
      bool   mqtt_retainFlag      = CPlugin_006_mqtt_retainFlag;

      statusLED(true);

      //LoadTaskSettings(event->TaskIndex); // FIXME TD-er: This can probably be removed

      const uint8_t valueCount = getValueCountForTask(event->TaskIndex);

      for (uint8_t x = 0; x < valueCount; x++)
      {
        String tmppubname = pubname;
        if (contains_valname) {
          parseSingleControllerVariable(tmppubname, event, x, false);
        }
        parseControllerVariables(tmppubname, event, false);

        // Small optimization so we don't try to copy potentially large strings
        if (event->sensorType == Sensor_VType::SENSOR_TYPE_STRING) {
          if (MQTTpublish(event->ControllerIndex, event->TaskIndex, tmppubname.c_str(), event->String2.c_str(), mqtt_retainFlag))
            success = true;
        } else {
          if (MQTTpublish(event->ControllerIndex, event->TaskIndex, std::move(tmppubname), formatUserVarNoCheck(event, x), mqtt_retainFlag))
            success = true;
        }
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      processMQTTdelayQueue();
      delay(0);
      break;
    }

    default:
      break;
  }
  return success;
}

#endif // ifdef USES_C006
