#include "../PluginStructs/P146_data_struct.h"

#ifdef USES_P146

# include "../ControllerQueue/C016_queue_element.h"
# include "../Globals/C016_ControllerCache.h"
# include "../Globals/MQTT.h"


P146_data_struct::P146_data_struct()
{}

P146_data_struct::~P146_data_struct()
{}


uint32_t writeToMqtt(const String& str, bool send) {
  if (send) {
    MQTTclient.write(str);
  }
  return str.length();
}

uint32_t writeToMqtt(const char& c, bool send) {
  if (send) 
    MQTTclient.write(String(c));
  return 1;
}

uint32_t createTaskInfoJson(bool send) {
  size_t expected_size = 0;
  expected_size += writeToMqtt('[', send);

  for (taskIndex_t task = 0; validTaskIndex(task); ++task) {
    {
      String taskName;
      if (task != 0) {
        taskName = ',';
      }
      taskName += concat(F("{\"taskName\":\""), getTaskDeviceName(task));
      taskName += concat(F("\",\"taskIndex\":"), task);
      taskName += F(",\"taskValues\":[");
      expected_size += writeToMqtt(taskName, send);
    }
    for (taskVarIndex_t rel_index = 0; rel_index < INVALID_TASKVAR_INDEX; ++rel_index) {
      String taskVarName;
      if (rel_index != 0) { 
        taskVarName = ',';
      }
      // FIXME TD-er: getTaskValueName returns empty string when task is not enabled, therefore use cache.
      //              Should getTaskValueName return empty string when task is disabled?
      taskVarName += wrap_String(Cache.getTaskDeviceValueName(task, rel_index), '"');
      expected_size += writeToMqtt(taskVarName, send);
    }
    {
      const String tail = F("]}");
      expected_size += writeToMqtt(tail, send);
    }
  }
  expected_size += writeToMqtt(']', send);
  return expected_size;
}

uint32_t P146_data_struct::sendTaskInfoInBulk(taskIndex_t P146_TaskIndex, uint32_t maxMessageSize)
{
  String topic = F("tracker_v2/%sysname%_%unit%/%tskname%/upload_meta");
  topic.replace(F("%tskname%"), getTaskDeviceName(P146_TaskIndex));
  topic = parseTemplate(topic);


  const size_t expected_size = createTaskInfoJson(false);
  MQTTclient.beginPublish(topic.c_str(), expected_size, false);
  createTaskInfoJson(true);
  MQTTclient.endPublish();
  return 0;
}

uint32_t P146_data_struct::sendBinaryInBulk(taskIndex_t P146_TaskIndex, uint32_t maxMessageSize)
{
  controllerIndex_t enabledMqttController = firstEnabledMQTT_ControllerIndex();

  if (!validControllerIndex(enabledMqttController)) {
    return 0;
  }


  // Keep the current peek position, so we can reset it when we fail to deliver the data to the controller.
  int peekFileNr        = 0;
  const int peekReadPos =  ControllerCache.getPeekFilePos(peekFileNr);


  String message;

  message += peekFileNr;
  message += ';';
  message += peekReadPos;
  message += ';';

  size_t messageLength = message.length();

  const size_t chunkSize           = sizeof(C016_binary_element);
  const size_t nrChunks            = (maxMessageSize - messageLength) / ((2 * chunkSize) + 1);
  const size_t expectedMessageSize = messageLength + (nrChunks * ((2 * chunkSize) + 1));

  if (0 == message.reserve(expectedMessageSize)) { return 0; }

  bool done = false;

  for (int chunk = 0; chunk < nrChunks && !done; ++chunk) {
    C016_binary_element element;

    if (ControllerCache.peek(reinterpret_cast<uint8_t *>(&element), chunkSize))
    {
      // It makes no sense to keep the controller index when storing it.
      element.setPluginID_insteadOf_controller_idx();
      message += formatToHex_array(reinterpret_cast<const uint8_t *>(&element), chunkSize);
      message += ';';
    } else {
      done = true;
    }
  }

  if (message.length() == messageLength) {
    // Nothing added, don't bother sending the peek pos
    return 0;
  }
  messageLength = message.length();

  String topic = F("tracker_v2/%sysname%_%unit%/%tskname%/upload");
  topic.replace(F("%tskname%"), getTaskDeviceName(P146_TaskIndex));
  topic = parseTemplate(topic);

  if (MQTTpublish(enabledMqttController, P146_TaskIndex, std::move(topic), std::move(message), false)) {
    return messageLength;
  }

  // Restore peek position
  ControllerCache.setPeekFilePos(peekFileNr, peekReadPos);
  return 0;
}

bool P146_data_struct::sendViaOriginalTask(
  taskIndex_t P146_TaskIndex, bool sendTimestamp)
{
  // Keep the current peek position, so we can reset it when we fail to deliver the data to the controller.
  int peekFileNr        = 0;
  const int peekReadPos =  ControllerCache.getPeekFilePos(peekFileNr);


  unsigned long timestamp;
  uint8_t valueCount;
  float   taskValues[VARS_PER_TASK] = {};
  EventStruct tmpEvent(C016_getTaskSample(
                         timestamp, valueCount,
                         taskValues[0], taskValues[1], taskValues[2], taskValues[3]));

  if (!validTaskIndex(tmpEvent.TaskIndex)) {
    return false;
  }

  bool success        = false;
  bool unusableSample = false;

  // Send the task values to a controller configured in this Cache Controller Reader task.
  // However we act as if we're sending from the original task to use the taskname and taskvalue names of that task.
  // We can't use the controllers linked in the original task, since this will probably
  // send the values again to the Cache Controller as they did in the first place.

  // Steps to take to flush it via the original tasks:
  // - Temporary copy the existing task values
  // - Copy values from the cache bin files to the task it had
  // - Call CPLUGIN_PROTOCOL_SEND
  // - Restore the values
  for (int valIndex = 0; valIndex < VARS_PER_TASK; ++valIndex) {
    std::swap(taskValues[valIndex], UserVar[tmpEvent.BaseVarIndex + valIndex]);
  }

  for (controllerIndex_t x = 0; !unusableSample && x < CONTROLLER_MAX; x++)
  {
    // Make sure we are not sending to the cache controller.
    if (Settings.Protocol[x] != 16) {
      tmpEvent.ControllerIndex = x;
      tmpEvent.idx             = Settings.TaskDeviceID[x][P146_TaskIndex];

      if (Settings.TaskDeviceSendData[x][P146_TaskIndex] &&
          Settings.ControllerEnabled[x] &&
          Settings.Protocol[x])
      {
        protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(x);

        if (validUserVar(&tmpEvent)) {
          String dummy;

          // FIXME TD-er: Must think about how to include the timestamp
          if (CPluginCall(ProtocolIndex, CPlugin::Function::CPLUGIN_PROTOCOL_SEND, &tmpEvent, dummy)) {
            // FIXME TD-er: What to do when multiple controllers are selected and one fails?
            // Also, what if we only sent 1 value instead of all?
            success = true;
          }
        } else {
          unusableSample = true;
        }
      }
    }
  }

  for (int valIndex = 0; valIndex < VARS_PER_TASK; ++valIndex) {
    std::swap(taskValues[valIndex], UserVar[tmpEvent.BaseVarIndex + valIndex]);
  }

  if (!success && !unusableSample) {
    // May need to retry later using this sample
    ControllerCache.setPeekFilePos(peekFileNr, peekReadPos);
  }
  return success;
}

bool P146_data_struct::setPeekFilePos(int peekFileNr, int peekReadPos)
{
  {
    const int modulo_24 = peekReadPos % sizeof(C016_binary_element);

    if (modulo_24 != 0) { peekReadPos -= modulo_24; }
  }

  ControllerCache.setPeekFilePos(peekFileNr, peekReadPos);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("CacheReader : SetReadPos,"), peekFileNr) + ',' + peekReadPos);
  }

  return true;
}

#endif // ifdef USES_P146
