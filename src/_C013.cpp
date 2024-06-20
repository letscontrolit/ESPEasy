#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C013

# if FEATURE_ESPEASY_P2P == 0
  #  error "Controller C013 ESPEasy P2P requires the FEATURE_ESPEASY_P2P enabled"
# endif // if FEATURE_ESPEASY_P2P == 0


# include "src/Globals/Nodes.h"
# include "src/DataStructs/C013_p2p_SensorDataStruct.h"
# include "src/DataStructs/C013_p2p_SensorInfoStruct.h"
# include "src/ESPEasyCore/ESPEasyRules.h"
# include "src/Helpers/Misc.h"
# include "src/Helpers/Network.h"

// #######################################################################################################
// ########################### Controller Plugin 013: ESPEasy P2P network ################################
// #######################################################################################################

# define CPLUGIN_013
# define CPLUGIN_ID_013         13
# define CPLUGIN_NAME_013       "ESPEasy P2P Networking"


// Forward declarations
void C013_SendUDPTaskInfo(uint8_t destUnit,
                          uint8_t sourceTaskIndex,
                          uint8_t destTaskIndex);
void C013_SendUDPTaskData(struct EventStruct *event,
                          uint8_t             destUnit,
                          uint8_t             destTaskIndex);
void C013_sendUDP(uint8_t        unit,
                  const uint8_t *data,
                  size_t         size);
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
  infoReply.destUnit        = destUnit;

  if (destUnit == 0)
  {
    // Send to broadcast address
    infoReply.destUnit = 255;
  }
  size_t sizeToSend{};

  if (infoReply.prepareForSend(sizeToSend)) {
    C013_sendUDP(infoReply.destUnit, reinterpret_cast<const uint8_t *>(&infoReply), sizeToSend);
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

  const TaskValues_Data_t *taskValues = UserVar.getRawTaskValues_Data(event->TaskIndex);

  if (taskValues != nullptr) {
    for (taskVarIndex_t x = 0; x < VARS_PER_TASK; ++x)
    {
      dataReply.values.copyValue(*taskValues, x, dataReply.sensorType);
    }
  }
  dataReply.destUnit = destUnit;

  if (destUnit == 0)
  {
    // Send to broadcast address
    dataReply.destUnit = 255;
  }
  dataReply.prepareForSend();
  C013_sendUDP(dataReply.destUnit, reinterpret_cast<const uint8_t *>(&dataReply), sizeof(C013_SensorDataStruct));
}

/*********************************************************************************************\
   Send UDP message (unit 255=broadcast)
\*********************************************************************************************/
void C013_sendUDP(uint8_t unit, const uint8_t *data, size_t size)
{
  START_TIMER

  if (!NetworkConnected(10)) {
    return;
  }

  const IPAddress remoteNodeIP = getIPAddressForUnit(unit);


# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    addLogMove(LOG_LEVEL_DEBUG_MORE, strformat(
                 F("C013 : Send UDP message to %d (%s)"),
                 unit,
                 remoteNodeIP.toString().c_str()));
  }
# endif // ifndef BUILD_NO_DEBUG

  statusLED(true);

  WiFiUDP C013_portUDP;

  if (!beginWiFiUDP_randomPort(C013_portUDP)) {
    STOP_TIMER(C013_SEND_UDP_FAIL);
    return;
  }

  FeedSW_watchdog();

  if (C013_portUDP.beginPacket(remoteNodeIP, Settings.UDPPort) == 0) {
    STOP_TIMER(C013_SEND_UDP_FAIL);
    return;
  }
  C013_portUDP.write(data, size);
  C013_portUDP.endPacket();
  C013_portUDP.stop();
  FeedSW_watchdog();
  delay(0);
  STOP_TIMER(C013_SEND_UDP);
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

  START_TIMER

  switch (event->Data[1]) {
    case 2: // sensor info pull request
    {
      // SendUDPTaskInfo(packetBuffer[2], packetBuffer[5], packetBuffer[4]);
      break;
    }

    case 3: // sensor info
    {
      bool mustSave         = false;
      taskIndex_t taskIndex = INVALID_TASK_INDEX;
      {
        // Allocate this is a separate scope since C013_SensorInfoStruct is a HUGE object
        // Should not be left allocated on the stack when calling PLUGIN_INIT and save, etc.
        struct C013_SensorInfoStruct infoReply;

        if (infoReply.setData(event->Data, event->Par2)) {
          // to prevent flash wear out (bugs in communication?) we can only write to an empty task
          // so it will write only once and has to be cleared manually through webgui
          // Also check the receiving end does support the plugin ID.
          const pluginID_t currentPluginID = Settings.getPluginID_for_task(infoReply.destTaskIndex);
          bool mustUpdateCurrentTask       = false;

          if (currentPluginID == infoReply.deviceNumber) {
            // Check to see if task already is set to receive from this host
            if ((Settings.TaskDeviceDataFeed[infoReply.destTaskIndex] == infoReply.sourceUnit) &&
                Settings.TaskDeviceEnabled[infoReply.destTaskIndex]) {
              mustUpdateCurrentTask = true;
            }
          }

          if ((mustUpdateCurrentTask || !validPluginID_fullcheck(currentPluginID)) &&
              supportedPluginID(infoReply.deviceNumber))
          {
            taskClear(infoReply.destTaskIndex, false);
            Settings.TaskDeviceNumber[infoReply.destTaskIndex]   = infoReply.deviceNumber.value;
            Settings.TaskDeviceDataFeed[infoReply.destTaskIndex] = infoReply.sourceUnit; // remote feed store unit nr sending the data

            if (mustUpdateCurrentTask) {
              Settings.TaskDeviceEnabled[infoReply.destTaskIndex] = true;
            }

            constexpr pluginID_t DUMMY_PLUGIN_ID{ 33 };

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

            if (infoReply.sourceNodeBuild >= 20871) {
              ExtraTaskSettings.version = infoReply.ExtraTaskSettings_version;

              for (uint8_t x = 0; x < VARS_PER_TASK; x++) {
//                safe_strncpy(ExtraTaskSettings.TaskDeviceFormula[x], infoReply.TaskDeviceFormula[x], sizeof(infoReply.TaskDeviceFormula[x]));
                ExtraTaskSettings.TaskDeviceValueDecimals[x] = infoReply.TaskDeviceValueDecimals[x];
                ExtraTaskSettings.TaskDeviceMinValue[x]      = infoReply.TaskDeviceMinValue[x];
                ExtraTaskSettings.TaskDeviceMaxValue[x]      = infoReply.TaskDeviceMaxValue[x];
                ExtraTaskSettings.TaskDeviceErrorValue[x]    = infoReply.TaskDeviceErrorValue[x];
                ExtraTaskSettings.VariousBits[x]             = infoReply.VariousBits[x];
              }

              for (uint8_t x = 0; x < PLUGIN_CONFIGVAR_MAX; ++x) {
                Settings.TaskDevicePluginConfig[infoReply.destTaskIndex][x] = infoReply.TaskDevicePluginConfig[x];
              }
            }

            ExtraTaskSettings.TaskIndex = infoReply.destTaskIndex;
            taskIndex                   = infoReply.destTaskIndex;
            mustSave                    = true;
          }
        }
      }

      if (mustSave) {
        SaveTaskSettings(taskIndex);
        SaveSettings();

        if (Settings.TaskDeviceEnabled[taskIndex]) {
          struct EventStruct TempEvent(taskIndex);
          TempEvent.Source = EventValueSource::Enum::VALUE_SOURCE_UDP;

          String dummy;
          PluginCall(PLUGIN_INIT, &TempEvent, dummy);
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

      // FIXME TD-er: We should check for sensorType and pluginID on both sides.
      // For example sending different sensor type data from one dummy to another is probably not going to work well

      if (dataReply.setData(event->Data, event->Par2)) {
        // only if this task has a remote feed, update values
        const uint8_t remoteFeed = Settings.TaskDeviceDataFeed[dataReply.destTaskIndex];

        if ((remoteFeed != 0) && (remoteFeed == dataReply.sourceUnit))
        {
          // deviceNumber and sensorType were not present before build 2023-05-05. (build NR 20460)
          // See:
          // https://github.com/letscontrolit/ESPEasy/commit/cf791527eeaf31ca98b07c45c1b64e2561a7b041#diff-86b42dd78398b103e272503f05f55ee0870ae5fb907d713c2505d63279bb0321
          // Thus should not be checked
          //
          // If the node is not present in the nodes list (e.g. it had not announced itself in the last 10 minutes or announcement was
          // missed)
          // Then we cannot be sure about its build.
          const bool mustMatch = dataReply.sourceNodeBuild >= 20460;

          if (mustMatch && !dataReply.matchesPluginID(Settings.getPluginID_for_task(dataReply.destTaskIndex))) {
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

            if (!mustMatch || dataReply.matchesSensorType(sensorType)) {
              TaskValues_Data_t *taskValues = UserVar.getRawTaskValues_Data(dataReply.destTaskIndex);

              if (taskValues != nullptr) {
                for (taskVarIndex_t x = 0; x < VARS_PER_TASK; ++x)
                {
                  taskValues->copyValue(dataReply.values, x, sensorType);
                }
              }
              STOP_TIMER(C013_RECEIVE_SENSOR_DATA);

              if (node_time.systemTimePresent() && (dataReply.timestamp_sec != 0)) {
                // Only use timestamp of remote unit when we got a system time ourselves
                // If not, then the order of samples can get messed up.
                SensorSendTask(&TempEvent, dataReply.timestamp_sec);
              } else {
                SensorSendTask(&TempEvent);
              }
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
