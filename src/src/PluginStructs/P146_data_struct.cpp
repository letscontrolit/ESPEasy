#include "../PluginStructs/P146_data_struct.h"

#ifdef USES_P146

# include "../Globals/C016_ControllerCache.h"

# include "../ControllerQueue/C016_queue_element.h"


P146_data_struct::P146_data_struct()
{}

P146_data_struct::~P146_data_struct()
{}


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

  const size_t chunkSize           = sizeof(C016_queue_element);
  const size_t nrChunks            = (maxMessageSize - messageLength) / ((2 * chunkSize) + 1);
  const size_t expectedMessageSize = messageLength + (nrChunks * ((2 * chunkSize) + 1));

  if (0 == message.reserve(expectedMessageSize)) { return 0; }

  bool done = false;

  for (int chunk = 0; chunk < nrChunks && !done; ++chunk) {
    uint8_t chunkdata[chunkSize] = { 0 };

    if (ControllerCache.peek(chunkdata, chunkSize))
    {
      message += formatToHex_array(chunkdata, chunkSize);
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
  String topic = F("CULreader/upload");

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
    const int modulo_24 = peekReadPos % sizeof(C016_queue_element);

    if (modulo_24 != 0) { peekReadPos -= modulo_24; }
  }

  ControllerCache.setPeekFilePos(peekFileNr, peekReadPos);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(F("CacheReader : SetReadPos,"), peekFileNr) + ',' + peekReadPos);
  }

  return true;
}

#endif // ifdef USES_P146
