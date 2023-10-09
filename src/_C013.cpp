#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C013

# if FEATURE_ESPEASY_P2P == 0
  #  error "Controller C013 ESPEasy P2P requires the FEATURE_ESPEASY_P2P enabled"
# endif // if FEATURE_ESPEASY_P2P == 0


# include "src/Globals/Nodes.h"
# include "src/DataStructs/C013_p2p_dataStructs.h"
# include "src/ESPEasyCore/ESPEasyRules.h"
# include "src/Helpers/Misc.h"
# include "src/Helpers/Network.h"

// #######################################################################################################
// ########################### Controller Plugin 013: ESPEasy P2P network ################################
// #######################################################################################################

# define CPLUGIN_013
# define CPLUGIN_ID_013         13
# define CPLUGIN_NAME_013       "ESPEasy P2P Networking"

WiFiUDP C013_portUDP;

// Forward declarations
void C013_SendUDPTaskInfo(uint8_t destUnit,
                          uint8_t sourceTaskIndex,
                          uint8_t destTaskIndex);
void C013_SendUDPTaskData(struct EventStruct *event,
                          uint8_t             destUnit,
                          uint8_t             destTaskIndex);
void C013_sendUDP(uint8_t        unit,
                  const uint8_t *data,
                  uint8_t        size);
void C013_Receive(struct EventStruct *event);


bool CPlugin_013(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      ProtocolStruct& proto = getProtocolStruct(event->idx); //      = CPLUGIN_ID_013;
      proto.usesMQTT     = false;
      proto.usesTemplate = false;
      proto.usesAccount  = false;
      proto.usesPassword = false;
      proto.usesHost     = false;
      proto.defaultPort  = 8266;
      proto.usesID       = false;
      proto.Custom       = true;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_013);
      break;
    }

    case CPlugin::Function::CPLUGIN_TASK_CHANGE_NOTIFICATION:
    {
      C013_SendUDPTaskInfo(0, event->TaskIndex, event->TaskIndex);
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      C013_SendUDPTaskData(event, 0, event->TaskIndex);
      success = true;
      break;
    }

    case CPlugin::Function::CPLUGIN_UDP_IN:
    {
      C013_Receive(event);
      break;
    }

    case CPlugin::Function::CPLUGIN_WEBFORM_SHOW_HOST_CONFIG:
    {
      string = F("-");
      break;
    }

    /*
        case CPlugin::Function::CPLUGIN_FLUSH:
          {
            process_c013_delay_queue(event->ControllerIndex);
            delay(0);
            break;
          }
     */

    default:
      break;
  }
  return success;
}

// ********************************************************************************
// Generic UDP message
// ********************************************************************************
void C013_SendUDPTaskInfo(uint8_t destUnit, uint8_t sourceTaskIndex, uint8_t destTaskIndex)
{
  if (!NetworkConnected(10)) {
    return;
  }

  if (!validTaskIndex(sourceTaskIndex) || !validTaskIndex(destTaskIndex)) {
    return;
  }
  pluginID_t pluginID = Settings.getPluginID_for_task(sourceTaskIndex);

  if (!validPluginID_fullcheck(pluginID)) {
    return;
  }

  struct C013_SensorInfoStruct infoReply;

  infoReply.sourceUnit      = Settings.Unit;
  infoReply.sourceTaskIndex = sourceTaskIndex;
  infoReply.destTaskIndex   = destTaskIndex;
  infoReply.deviceNumber    = pluginID;
  safe_strncpy(infoReply.taskName, getTaskDeviceName(infoReply.sourceTaskIndex), sizeof(infoReply.taskName));

  for (uint8_t x = 0; x < VARS_PER_TASK; x++) {
    safe_strncpy(infoReply.ValueNames[x], getTaskValueName(infoReply.sourceTaskIndex, x), sizeof(infoReply.ValueNames[x]));
  }

  if (destUnit != 0)
  {
    infoReply.destUnit = destUnit;
    C013_sendUDP(destUnit, reinterpret_cast<const uint8_t *>(&infoReply), sizeof(C013_SensorInfoStruct));
  } else {
    for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
      if (it->first != Settings.Unit) {
        infoReply.destUnit = it->first;
        C013_sendUDP(it->first, reinterpret_cast<const uint8_t *>(&infoReply), sizeof(C013_SensorInfoStruct));
      }
    }
  }
}

void C013_SendUDPTaskData(struct EventStruct *event, uint8_t destUnit, uint8_t destTaskIndex)
{
  if (!NetworkConnected(10)) {
    return;
  }
  struct C013_SensorDataStruct dataReply;

  dataReply.sourceUnit      = Settings.Unit;
  dataReply.sourceTaskIndex = event->TaskIndex;
  dataReply.destTaskIndex   = destTaskIndex;
  dataReply.deviceNumber    = Settings.getPluginID_for_task(event->TaskIndex);

  // FIXME TD-er: We should check for sensorType and pluginID on both sides.
  // For example sending different sensor type data from one dummy to another is probably not going to work well
  dataReply.sensorType = event->getSensorType();

  const TaskValues_Data_t *taskValues = UserVar.getTaskValues_Data(event->TaskIndex);

  if (taskValues != nullptr) {
    for (taskVarIndex_t x = 0; x < VARS_PER_TASK; ++x)
    {
      dataReply.values.copyValue(*taskValues, x, dataReply.sensorType);
    }
  }

  if (destUnit != 0)
  {
    dataReply.destUnit = destUnit;
    C013_sendUDP(destUnit, reinterpret_cast<const uint8_t *>(&dataReply), sizeof(C013_SensorDataStruct));
  } else {
    for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
      if (it->first != Settings.Unit) {
        dataReply.destUnit = it->first;
        C013_sendUDP(it->first, reinterpret_cast<const uint8_t *>(&dataReply), sizeof(C013_SensorDataStruct));
      }
    }
  }
}

/*********************************************************************************************\
   Send UDP message (unit 255=broadcast)
\*********************************************************************************************/
void C013_sendUDP(uint8_t unit, const uint8_t *data, uint8_t size)
{
  if (!NetworkConnected(10)) {
    return;
  }

# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    addLogMove(LOG_LEVEL_DEBUG_MORE, concat(F("C013 : Send UDP message to "), unit));
  }
# endif // ifndef BUILD_NO_DEBUG

  statusLED(true);

  if (!beginWiFiUDP_randomPort(C013_portUDP)) { return; }

  FeedSW_watchdog();
  const IPAddress remoteNodeIP = getIPAddressForUnit(unit);

  if (C013_portUDP.beginPacket(remoteNodeIP, Settings.UDPPort) == 0) { return; }
  C013_portUDP.write(data, size);
  C013_portUDP.endPacket();
  C013_portUDP.stop();
  FeedSW_watchdog();
  delay(0);
}

void C013_Receive(struct EventStruct *event) {
  if (event->Par2 < 6) { return; }
# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    if ((event->Data != nullptr) &&
        (event->Data[1] > 1) && (event->Data[1] < 6))
    {
      String log = (F("C013 : msg "));

      for (uint8_t x = 1; x < 6; x++)
      {
        log += ' ';
        log += static_cast<int>(event->Data[x]);
      }
      addLogMove(LOG_LEVEL_DEBUG_MORE, log);
    }
  }
# endif // ifndef BUILD_NO_DEBUG

  switch (event->Data[1]) {
    case 2: // sensor info pull request
    {
      // SendUDPTaskInfo(packetBuffer[2], packetBuffer[5], packetBuffer[4]);
      break;
    }

    case 3: // sensor info
    {
      struct C013_SensorInfoStruct infoReply;
      int structSize = sizeof(C013_SensorInfoStruct);

      if (event->Par2 < structSize) { structSize = event->Par2; }

      memcpy(reinterpret_cast<uint8_t *>(&infoReply), event->Data, structSize);

      if (infoReply.isValid()) {
        // to prevent flash wear out (bugs in communication?) we can only write to an empty task
        // so it will write only once and has to be cleared manually through webgui
        // Also check the receiving end does support the plugin ID.
        if (!validPluginID_fullcheck(Settings.getPluginID_for_task(infoReply.destTaskIndex)) &&
            supportedPluginID(infoReply.deviceNumber))
        {
          taskClear(infoReply.destTaskIndex, false);
          Settings.TaskDeviceNumber[infoReply.destTaskIndex]   = infoReply.deviceNumber.value;
          Settings.TaskDeviceDataFeed[infoReply.destTaskIndex] = infoReply.sourceUnit; // remote feed store unit nr sending the data

          constexpr pluginID_t DUMMY_PLUGIN_ID{33};
          if ((infoReply.deviceNumber == DUMMY_PLUGIN_ID) && (infoReply.sensorType != Sensor_VType::SENSOR_TYPE_NONE)) {
            // Received a dummy device and the sensor type is actually set
            Settings.TaskDevicePluginConfig[infoReply.destTaskIndex][0] = static_cast<int16_t>(infoReply.sensorType);
          }

          for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
            Settings.TaskDeviceSendData[x][infoReply.destTaskIndex] = false;
          }
          safe_strncpy(ExtraTaskSettings.TaskDeviceName, infoReply.taskName, sizeof(infoReply.taskName));

          for (uint8_t x = 0; x < VARS_PER_TASK; x++) {
            safe_strncpy(ExtraTaskSettings.TaskDeviceValueNames[x], infoReply.ValueNames[x], sizeof(infoReply.ValueNames[x]));
          }
          ExtraTaskSettings.TaskIndex = infoReply.destTaskIndex;
          SaveTaskSettings(infoReply.destTaskIndex);
          SaveSettings();
        }
      }
      break;
    }

    case 4: // sensor data pull request
    {
      // SendUDPTaskData(packetBuffer[2], packetBuffer[5], packetBuffer[4]);
      break;
    }

    case 5: // sensor data
    {
      struct C013_SensorDataStruct dataReply;
      int structSize = sizeof(C013_SensorDataStruct);

      if (event->Par2 < structSize) { structSize = event->Par2; }
      memcpy(reinterpret_cast<uint8_t *>(&dataReply), event->Data, structSize);

      // FIXME TD-er: We should check for sensorType and pluginID on both sides.
      // For example sending different sensor type data from one dummy to another is probably not going to work well
      if (dataReply.isValid()) {
        // only if this task has a remote feed, update values
        const uint8_t remoteFeed = Settings.TaskDeviceDataFeed[dataReply.destTaskIndex];

        if ((remoteFeed != 0) && (remoteFeed == dataReply.sourceUnit))
        {
          if (!dataReply.matchesPluginID(Settings.getPluginID_for_task(dataReply.destTaskIndex))) {
            // Mismatch in plugin ID from sending node
            if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
              String log = concat(F("P2P data : PluginID mismatch for task "), dataReply.destTaskIndex + 1);
              log += concat(F(" from unit "), dataReply.sourceUnit);
              log += concat(F(" remote: "), dataReply.deviceNumber.value);
              log += concat(F(" local: "), Settings.getPluginID_for_task(dataReply.destTaskIndex).value);
              addLogMove(LOG_LEVEL_ERROR, log);
            }
          } else {
            struct EventStruct TempEvent(dataReply.destTaskIndex);
            TempEvent.Source = EventValueSource::Enum::VALUE_SOURCE_UDP;

            const Sensor_VType sensorType = TempEvent.getSensorType();

            if (dataReply.matchesSensorType(sensorType)) {
              TaskValues_Data_t *taskValues = UserVar.getTaskValues_Data(dataReply.destTaskIndex);

              if (taskValues != nullptr) {
                for (taskVarIndex_t x = 0; x < VARS_PER_TASK; ++x)
                {
                  taskValues->copyValue(dataReply.values, x, sensorType);
                }
              }

              SensorSendTask(&TempEvent);
            } else {
              // Mismatch in sensor types
              if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
                String log = concat(F("P2P data : SensorType mismatch for task "), dataReply.destTaskIndex + 1);
                log += concat(F(" from unit "), dataReply.sourceUnit);
                addLogMove(LOG_LEVEL_ERROR, log);
              }
            }
          }
        }
      }
      break;
    }
  }
}

#endif // ifdef USES_C013
