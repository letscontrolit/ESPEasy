#ifdef USES_C013
//#######################################################################################################
//########################### Controller Plugin 013: ESPEasy P2P network ################################
//#######################################################################################################

#define CPLUGIN_013
#define CPLUGIN_ID_013         13
#define CPLUGIN_NAME_013       "ESPEasy P2P Networking"

WiFiUDP C013_portUDP;

struct C013_SensorInfoStruct
{
  byte header = 255;
  byte ID = 3;
  byte sourcelUnit;
  byte destUnit;
  byte sourceTaskIndex;
  byte destTaskIndex;
  byte deviceNumber;
  char taskName[26];
  char ValueNames[VARS_PER_TASK][26];
};

struct C013_SensorDataStruct
{
  byte header = 255;
  byte ID = 5;
  byte sourcelUnit;
  byte destUnit;
  byte sourceTaskIndex;
  byte destTaskIndex;
  float Values[VARS_PER_TASK];
};


bool CPlugin_013(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_013;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesTemplate = false;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = false;
        Protocol[protocolCount].defaultPort = 65501;
        Protocol[protocolCount].usesID = false;
        Protocol[protocolCount].Custom = true;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_013);
        break;
      }

    case CPLUGIN_PROTOCOL_TEMPLATE:
      {
        event->String1 = "";
        event->String2 = "";
        break;
      }

    case CPLUGIN_INIT:
      {
        //C013_portUDP.begin(Settings.UDPPort);
        break;
      }

    case CPLUGIN_TASK_CHANGE_NOTIFICATION:
      {
        C013_SendUDPTaskInfo(0, event->TaskIndex, event->TaskIndex);
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        C013_SendUDPTaskData(0, event->TaskIndex, event->TaskIndex);
        break;
      }

    case CPLUGIN_UDP_IN:
      {
        C013_Receive(event);
        break;
      }

  }
  return success;
}


//********************************************************************************
// Generic UDP message
//********************************************************************************
void C013_SendUDPTaskInfo(byte destUnit, byte sourceTaskIndex, byte destTaskIndex)
{
  if (!WiFiConnected(100)) {
    return;
  }
  struct C013_SensorInfoStruct infoReply;
  infoReply.sourcelUnit = Settings.Unit;
  infoReply.sourceTaskIndex = sourceTaskIndex;
  infoReply.destTaskIndex = destTaskIndex;
  LoadTaskSettings(infoReply.sourceTaskIndex);
  infoReply.deviceNumber = Settings.TaskDeviceNumber[infoReply.sourceTaskIndex];
  strcpy(infoReply.taskName, getTaskDeviceName(infoReply.sourceTaskIndex).c_str());
  for (byte x = 0; x < VARS_PER_TASK; x++)
    strcpy(infoReply.ValueNames[x], ExtraTaskSettings.TaskDeviceValueNames[x]);

  if (destUnit != 0)
  {
    infoReply.destUnit = destUnit;
    C013_sendUDP(destUnit, (byte*)&infoReply, sizeof(C013_SensorInfoStruct));
    delay(10);
  } else {
    for (NodesMap::iterator it = Nodes.begin(); it != Nodes.end(); ++it) {
      if (it->first != Settings.Unit) {
        infoReply.destUnit = it->first;
        C013_sendUDP(it->first, (byte*)&infoReply, sizeof(C013_SensorInfoStruct));
        delay(10);
      }
    }
  }
  delay(50);
}

void C013_SendUDPTaskData(byte destUnit, byte sourceTaskIndex, byte destTaskIndex)
{
  if (!WiFiConnected(100)) {
    return;
  }
  struct C013_SensorDataStruct dataReply;
  dataReply.sourcelUnit = Settings.Unit;
  dataReply.sourceTaskIndex = sourceTaskIndex;
  dataReply.destTaskIndex = destTaskIndex;
  for (byte x = 0; x < VARS_PER_TASK; x++)
    dataReply.Values[x] = UserVar[dataReply.sourceTaskIndex * VARS_PER_TASK + x];

  if (destUnit != 0)
  {
    dataReply.destUnit = destUnit;
    C013_sendUDP(destUnit, (byte*) &dataReply, sizeof(C013_SensorDataStruct));
    delay(10);
  } else {
    for (NodesMap::iterator it = Nodes.begin(); it != Nodes.end(); ++it) {
      if (it->first != Settings.Unit) {
        dataReply.destUnit = it->first;
        C013_sendUDP(it->first, (byte*) &dataReply, sizeof(C013_SensorDataStruct));
        delay(10);
      }
    }
  }
  delay(50);
}

/*********************************************************************************************\
   Send UDP message (unit 255=broadcast)
  \*********************************************************************************************/
void C013_sendUDP(byte unit, byte* data, byte size)
{
  if (!WiFiConnected(100)) {
    return;
  }
  NodesMap::iterator it;
  if (unit != 255) {
    it = Nodes.find(unit);
    if (it == Nodes.end())
      return;
    if (it->second.ip[0] == 0)
      return;
  }
  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    String log = F("C013 : Send UDP message to ");
    log += unit;
    addLog(LOG_LEVEL_DEBUG_MORE, log);
  }

  statusLED(true);

  IPAddress remoteNodeIP;
  if (unit == 255)
    remoteNodeIP = {255, 255, 255, 255};
  else
    remoteNodeIP = it->second.ip;
  if (!beginWiFiUDP_randomPort(C013_portUDP)) return;
  if (C013_portUDP.beginPacket(remoteNodeIP, Settings.UDPPort) == 0) return;
  C013_portUDP.write(data, size);
  C013_portUDP.endPacket();
  C013_portUDP.stop();
}

void C013_Receive(struct EventStruct *event) {
  if (event->Par2 < 6) return;
  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    if (event->Data[1] > 1 && event->Data[1] < 6)
    {
      String log = (F("C013 : msg "));
      for (byte x = 1; x < 6; x++)
      {
        log += ' ';
        log += (int)event->Data[x];
      }
      addLog(LOG_LEVEL_DEBUG_MORE, log);
    }
  }

  switch (event->Data[1]) {
    case 2: // sensor info pull request
      {
        //SendUDPTaskInfo(packetBuffer[2], packetBuffer[5], packetBuffer[4]);
        break;
      }

    case 3: // sensor info
      {
        struct C013_SensorInfoStruct infoReply;
        if (static_cast<size_t>(event->Par2) < sizeof(C013_SensorInfoStruct)) {
          addLog(LOG_LEVEL_DEBUG, F("C013_Receive: Received data smaller than C013_SensorInfoStruct, discarded"));
        } else {
          memcpy((byte*)&infoReply, (byte*)event->Data, sizeof(C013_SensorInfoStruct));

          // to prevent flash wear out (bugs in communication?) we can only write to an empty task
          // so it will write only once and has to be cleared manually through webgui
          if (Settings.TaskDeviceNumber[infoReply.destTaskIndex] == 0)
          {
            taskClear(infoReply.destTaskIndex, false);
            Settings.TaskDeviceNumber[infoReply.destTaskIndex] = infoReply.deviceNumber;
            Settings.TaskDeviceDataFeed[infoReply.destTaskIndex] = 1;  // remote feed
            for (byte x = 0; x < CONTROLLER_MAX; x++)
              Settings.TaskDeviceSendData[x][infoReply.destTaskIndex] = false;
            strcpy(ExtraTaskSettings.TaskDeviceName, infoReply.taskName);
            for (byte x = 0; x < VARS_PER_TASK; x++)
              strcpy( ExtraTaskSettings.TaskDeviceValueNames[x], infoReply.ValueNames[x]);
            ExtraTaskSettings.TaskIndex = infoReply.destTaskIndex;
            SaveTaskSettings(infoReply.destTaskIndex);
            SaveSettings();
          }
        }
        break;
      }

    case 4: // sensor data pull request
      {
        //SendUDPTaskData(packetBuffer[2], packetBuffer[5], packetBuffer[4]);
        break;
      }

    case 5: // sensor data
      {
        struct C013_SensorDataStruct dataReply;
        if (static_cast<size_t>(event->Par2) < sizeof(C013_SensorDataStruct)) {
          addLog(LOG_LEVEL_DEBUG, F("C013_Receive: Received data smaller than C013_SensorDataStruct, discarded"));
        } else {
          memcpy((byte*)&dataReply, (byte*)event->Data, sizeof(C013_SensorDataStruct));

          // only if this task has a remote feed, update values
          if (Settings.TaskDeviceDataFeed[dataReply.destTaskIndex] != 0)
          {
            for (byte x = 0; x < VARS_PER_TASK; x++)
            {
              UserVar[dataReply.destTaskIndex * VARS_PER_TASK + x] = dataReply.Values[x];
            }
            if (Settings.UseRules)
              createRuleEvents(dataReply.destTaskIndex);
          }
        }
        break;
      }
  }
}
#endif
