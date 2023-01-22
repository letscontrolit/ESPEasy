#include "../PluginStructs/P146_data_struct.h"

#ifdef USES_P146

# include "../ControllerQueue/C016_queue_element.h"
# include "../Globals/C016_ControllerCache.h"
# include "../Globals/MQTT.h"


P146_data_struct::P146_data_struct(taskIndex_t P146_TaskIndex)
{
  LoadCustomTaskSettings(P146_TaskIndex, _topics, P146_Nlines, 0);
}

P146_data_struct::~P146_data_struct()
{}


uint32_t writeToMqtt(const String& str, bool send) {
  if (send) {
    MQTTclient.write(str);
  }
  return str.length();
}

uint32_t writeToMqtt(const char& c, bool send) {
  if (send) {
    MQTTclient.write(static_cast<uint8_t>(c));
  }
  return 1;
}

uint32_t createTaskInfoJson(bool send) {
  size_t expected_size = 0;

  expected_size += writeToMqtt('[', send);

  for (taskIndex_t task = 0; validTaskIndex(task); ++task) {
    {
      if (task != 0) {
        expected_size += writeToMqtt(',', send);
      }
      expected_size += writeToMqtt(concat(F("{\"taskName\":\""), getTaskDeviceName(task)), send);
      expected_size += writeToMqtt(concat(F("\",\"taskIndex\":"), task), send);
      expected_size += writeToMqtt(concat(F(",\"pluginId\":"), getPluginID_from_TaskIndex(task)), send);
      expected_size += writeToMqtt(F(",\"taskValues\":["), send);
    }

    for (taskVarIndex_t rel_index = 0; rel_index < INVALID_TASKVAR_INDEX; ++rel_index) {
      if (rel_index != 0) {
        expected_size += writeToMqtt(',', send);
      }

      // FIXME TD-er: getTaskValueName returns empty string when task is not enabled, therefore use cache.
      //              Should getTaskValueName return empty string when task is disabled?
      expected_size += writeToMqtt(wrap_String(Cache.getTaskDeviceValueName(task, rel_index), '"'), send);
    }
    {
      const String tail = F("]}");
      expected_size += writeToMqtt(tail, send);
    }
  }
  expected_size += writeToMqtt(']', send);
  return expected_size;
}

uint32_t P146_data_struct::sendTaskInfoInBulk(taskIndex_t P146_TaskIndex, uint32_t maxMessageSize) const
{
  const String topic         = getTopic(P146_TaskInfoTopicIndex, P146_TaskIndex);
  const size_t expected_size = createTaskInfoJson(false);

  if (MQTTclient.beginPublish(topic.c_str(), expected_size, false)) {
    createTaskInfoJson(true);
    MQTTclient.endPublish();
  }
  return 0;
}

uint32_t P146_data_struct::sendBinaryInBulk(taskIndex_t P146_TaskIndex, uint32_t maxMessageSize) const
{
  const controllerIndex_t enabledMqttController = firstEnabledMQTT_ControllerIndex();

  if (!validControllerIndex(enabledMqttController)) {
    return 0;
  }


  // Keep the current peek position, so we can reset it when we fail to deliver the data to the controller.
  int peekFileNr  = 0;
  int peekReadPos =  ControllerCache.getPeekFilePos(peekFileNr);

  if (peekFileNr <= 0) {
    return 0;
  }

  int peekFileSize = ControllerCache.getPeekFileSize(peekFileNr);

  if (peekFileSize < 0) {
    // Peek file is not opened. Setting peek file pos will try to open it.
    ControllerCache.setPeekFilePos(peekFileNr, peekReadPos);
    peekReadPos  = ControllerCache.getPeekFilePos(peekFileNr);
    peekFileSize = ControllerCache.getPeekFileSize(peekFileNr);
  }

  // FIXME TD-er: This is messy, but needed to increment peek file nr.
  if (peekReadPos >= peekFileSize) {
    ControllerCache.setPeekFilePos(peekFileNr, peekReadPos);
    peekReadPos  = ControllerCache.getPeekFilePos(peekFileNr);
    peekFileSize = ControllerCache.getPeekFileSize(peekFileNr);
  }

  if (peekFileSize < 0) {
    // peek file still not open.
    return 0;
  }

  String message;

  message += peekFileNr;
  message += ';';
  message += peekReadPos;
  message += ';';

  const size_t messageLength = message.length();
  const size_t data_left     = (maxMessageSize - messageLength);
  const size_t chunkSize     = sizeof(C016_binary_element);
  size_t nrChunks            = data_left / ((2 * chunkSize) + 1);

  if (peekReadPos < peekFileSize) {
    // Try to serve the rest of the file in 1 go.
    const size_t chunksLeftInFile = (peekFileSize - peekReadPos) / chunkSize;

    if (chunksLeftInFile < nrChunks) {
      nrChunks = chunksLeftInFile;
    }
  }

  const size_t expectedMessageSize = messageLength + (nrChunks * ((2 * chunkSize) + 1));

  if (nrChunks == 0) {
    // Nothing to be sent
    return 0;
  }


  const String topic = getTopic(P146_PublishTopicIndex, P146_TaskIndex);

  if (!MQTTclient.beginPublish(topic.c_str(), expectedMessageSize, false)) {
    // Can't start a message
    return 0;
  }
  writeToMqtt(message, true);

  for (int chunk = 0; chunk < nrChunks; ++chunk) {
    C016_binary_element element;

    if (ControllerCache.peek(reinterpret_cast<uint8_t *>(&element), chunkSize))
    {
      // Test data to show layout of binary content:

      /*
            element._timestamp = 0x11223344;
            element.TaskIndex = 0x55;
            element.controller_idx = 0x66;
            element.sensorType = Sensor_VType::SENSOR_TYPE_NONE; // 0x00
            element.valueCount = 0x88;
            element.values[0] = 0x99;
            element.values[1] = 0xAA;
            element.values[2] = 0xBB;
            element.values[3] = 0xCC;
       */

      // Example of MQTT message containing a single CacheController sample:
      // Filenr;Filepos;Sample (24 bytes -> 48 HEX digits);
      // 17;14616;0000194300002a4300003b4300004c434433221155660088;

      // val[0]   val[1]   val[2]   val[3]   timestmp taskidx PluginID SensorType ValueCount
      // 00001943 00002a43 00003b43 00004c43 44332211 55      66       00         88
    }
    writeToMqtt(formatToHex_array(reinterpret_cast<const uint8_t *>(&element), chunkSize), true);
    writeToMqtt(';',                                                                       true);
  }
  MQTTclient.endPublish();

  // Restore peek position
  //  ControllerCache.setPeekFilePos(peekFileNr, peekReadPos);
  return expectedMessageSize;
}

bool P146_data_struct::prepareBinaryInBulk(taskIndex_t P146_TaskIndex, uint32_t messageSize)
{
  controllerIndex_t enabledMqttController = firstEnabledMQTT_ControllerIndex();

  if (!validControllerIndex(enabledMqttController)) {
    return 0;
  }

  // Just create an 'empty' controller queue message so we will use the controller's mechanism to limit the sending rate.
  String topic;
  String message;
  const bool callbackTask = true;

  return MQTTpublish(enabledMqttController, P146_TaskIndex, std::move(topic), std::move(message), false, callbackTask);
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

String P146_data_struct::getTopic(int index, taskIndex_t P146_TaskIndex) const {
  if ((index != P146_TaskInfoTopicIndex) && (index != P146_PublishTopicIndex)) {
    return EMPTY_STRING;
  }
  String topic = _topics[index];

  topic.replace(F("%tskname%"), getTaskDeviceName(P146_TaskIndex));
  topic = parseTemplate(topic);
  return topic;
}

#endif // ifdef USES_P146
