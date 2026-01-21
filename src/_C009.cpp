#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C009

# include "src/DataTypes/NodeTypeID.h"
# include "src/Helpers/StringProvider.h"
# include "src/Helpers/KeyValueWriter_JSON.h"

# include "src/CustomBuild/ESPEasy_buildinfo.h"

// #######################################################################################################
// ########################### Controller Plugin 009: FHEM HTTP ##########################################
// #######################################################################################################

/*******************************************************************************
 * Copyright 2016-2017 dev0
 * Contact: https://forum.fhem.de/index.php?action=profile;u=7465
 *          https://github.com/ddtlabs/
 *
 * Release notes:
   - v1.0
   - changed switch and dimmer setreading cmds
   - v1.01
   - added json content to http requests
   - v1.02
   - some optimizations as requested by mvdbro
   - fixed JSON TaskDeviceValueDecimals handling
   - ArduinoJson Library v5.6.4 required (as used by stable R120)
   - parse for HTTP errors 400, 401
   - moved on/off translation for Sensor_VType::SENSOR_TYPE_SWITCH/DIMMER to FHEM module
   - v1.03
   - changed http request from GET to POST (RFC conform)
   - removed obsolete http get url code
   - v1.04
   - added build options and node_type_id to JSON/device
 ******************************************************************************/

# define CPLUGIN_009
# define CPLUGIN_ID_009         9
# define CPLUGIN_NAME_009       "FHEM HTTP"

bool CPlugin_009(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      ProtocolStruct& proto = getProtocolStruct(event->idx); //      = CPLUGIN_ID_009;
      proto.usesMQTT     = false;
      proto.usesTemplate = false;
      proto.usesAccount  = true;
      proto.usesPassword = true;
      proto.usesExtCreds = true;
      proto.usesID       = false;
      proto.defaultPort  = 8383;
      # if FEATURE_HTTP_TLS
      proto.usesTLS = true;
      # endif // if FEATURE_HTTP_TLS
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_009);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_c009_delay_queue(event->ControllerIndex);
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      exit_c009_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      if (C009_DelayHandler != nullptr) {
        if (C009_DelayHandler->queueFull(event->ControllerIndex)) {
          break;
        }

        constexpr unsigned size = sizeof(C009_queue_element);
        void *ptr               = special_calloc(1, size);

        if (ptr != nullptr) {
          UP_C009_queue_element  element(new (ptr) C009_queue_element(event));
          success = C009_DelayHandler->addToQueue(std::move(element));
        }
        Scheduler.scheduleNextDelayQueue(SchedulerIntervalTimer_e::TIMER_C009_DELAY_QUEUE, C009_DelayHandler->getNextScheduleTime());
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      process_c009_delay_queue();
      delay(0);
      break;
    }

    default:
      break;
  }
  return success;
}

/*********************************************************************************************\
* FHEM HTTP request
\*********************************************************************************************/

// Uncrustify may change this into multi line, which will result in failed builds
// *INDENT-OFF*
bool do_process_c009_delay_queue(cpluginID_t cpluginID, const Queue_element_base& element_base, ControllerSettingsStruct& ControllerSettings) {
  const C009_queue_element& element = static_cast<const C009_queue_element&>(element_base);
// *INDENT-ON*

  // Make an educated guess on the actual length, based on earlier requests.
  static size_t expectedJsonLength = 100;
  PrintToString jsonPrint;
  jsonPrint.reserve(expectedJsonLength);
  {
    KeyValueWriter_JSON mainLevelWriter(true, &jsonPrint);
    {
      {
        mainLevelWriter.write({F("module"),  F("ESPEasy")});
        mainLevelWriter.write({F("version"), F("1.04")});

        // Create nested object "ESP" inside "data"
        auto data = mainLevelWriter.createChild(F("data"));
        if (data) {
          {
            auto esp = data->createChild(F("ESP"));
            if (esp)
            {
              // Create nested objects in "ESP":
              esp->write({F("name"), Settings.getName()});
              esp->write({F("unit"), static_cast<int>(Settings.Unit)});
              esp->write({F("version"), static_cast<int>(Settings.Version)});
              esp->write({F("build"), static_cast<int>(Settings.Build)});
              esp->write({F("build_notes"), F(BUILD_NOTES)});
              esp->write({F("build_git"), getValue(LabelType::GIT_BUILD)});
              esp->write({F("node_type_id"), static_cast<int>(NODE_TYPE_ID)});
              esp->write({F("sleep"), static_cast<int>(Settings.deepSleep_wakeTime)});

              // embed IP, important if there is NAT/PAT
              // char ipStr[20];
              // IPAddress ip = NetworkLocalIP();
              // sprintf_P(ipStr, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]});
              esp->write({F("ip"), formatIP(ESPEasy::net::NetworkLocalIP())});
            }
          } // End "ESP"
          {
            // Create nested object "SENSOR" json object inside "data"
            auto sensor = data->createChild(F("SENSOR"));
            if (sensor)
            {
              // char itemNames[valueCount][2];
              for (uint8_t x = 0; x < element.valueCount; x++)
              {
                // Each sensor value get an own object (0..n)
                // sprintf(itemNames[x],"%d",x);
                auto val_x = sensor->createChild(String(x));
                if (val_x) {
                  val_x->write({F("deviceName"), getTaskDeviceName(element._taskIndex)});
                  val_x->write({F("valueName"), Cache.getTaskDeviceValueName(element._taskIndex, x)});
                  val_x->write({F("type"), static_cast<int>(element.sensorType)});
                  val_x->write({F("value"), element.txt[x]});
                }
              } // End "sensor value N"          
            }
          } // End "SENSOR"
        }
      } // End "data"
    }
  } // End mainLevelWriter

  if (expectedJsonLength < jsonPrint.get().length()) {
    expectedJsonLength = jsonPrint.get().length();
  }

  // addLog(LOG_LEVEL_INFO, F("C009 Test JSON:"));
  // addLog(LOG_LEVEL_INFO, mainLevelWriter);

  int httpCode = -1;
  send_via_http(
    cpluginID,
    ControllerSettings,
    element._controller_idx,
    F("/ESPEasy"),
    F("POST"),
    EMPTY_STRING,
    jsonPrint.get(),
    httpCode);
  return (httpCode >= 100) && (httpCode < 300);
}

#endif // ifdef USES_C009
