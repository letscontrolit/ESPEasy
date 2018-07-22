#ifdef USES_C013
//#######################################################################################################
//########################### Controller Plugin 013: ESPEasy P2P network ################################
//#######################################################################################################

#define CPLUGIN_013
#define CPLUGIN_ID_013         13
#define CPLUGIN_NAME_013       "ESPEasy P2P Networking"

WiFiUDP C013_portUDP;

struct infoStruct
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

struct dataStruct
{
  byte header = 255;
  byte ID = 5;
  byte sourcelUnit;
  byte destUnit;
  byte sourceTaskIndex;
  byte destTaskIndex;
  float Values[VARS_PER_TASK];
};


boolean CPlugin_013(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

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
        C013_Send(event, 0, UserVar[event->BaseVarIndex], 0);
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
void C013_Send(struct EventStruct *event, byte varIndex, float value, unsigned long longValue)
{
  ControllerSettingsStruct ControllerSettings;
  LoadControllerSettings(event->ControllerIndex, (byte*)&ControllerSettings, sizeof(ControllerSettings));
  statusLED(true);
  C013_SendUDPTaskData(0, event->TaskIndex, event->TaskIndex);
}

void C013_SendUDPTaskInfo(byte destUnit, byte sourceTaskIndex, byte destTaskIndex)
{
  if (!WiFiConnected(100)) {
    return;
  }
  struct infoStruct infoReply;
  infoReply.sourcelUnit = Settings.Unit;
  infoReply.sourceTaskIndex = sourceTaskIndex;
  infoReply.destTaskIndex = destTaskIndex;
  LoadTaskSettings(infoReply.sourceTaskIndex);
  infoReply.deviceNumber = Settings.TaskDeviceNumber[infoReply.sourceTaskIndex];
  strcpy(infoReply.taskName, ExtraTaskSettings.TaskDeviceName);
  for (byte x = 0; x < VARS_PER_TASK; x++)
    strcpy(infoReply.ValueNames[x], ExtraTaskSettings.TaskDeviceValueNames[x]);

  byte firstUnit = 1;
  byte lastUnit = UNIT_MAX - 1;
  if (destUnit != 0)
  {
    firstUnit = destUnit;
    lastUnit = destUnit;
  }
  for (byte x = firstUnit; x <= lastUnit; x++)
  {
    if (x != Settings.Unit){
      infoReply.destUnit = x;
      C013_sendUDP(x, (byte*)&infoReply, sizeof(infoStruct));
      delay(10);
    }
  }
  delay(50);
}

void C013_SendUDPTaskData(byte destUnit, byte sourceTaskIndex, byte destTaskIndex)
{
  if (!WiFiConnected(100)) {
    return;
  }
  struct dataStruct dataReply;
  dataReply.sourcelUnit = Settings.Unit;
  dataReply.sourceTaskIndex = sourceTaskIndex;
  dataReply.destTaskIndex = destTaskIndex;
  for (byte x = 0; x < VARS_PER_TASK; x++)
    dataReply.Values[x] = UserVar[dataReply.sourceTaskIndex * VARS_PER_TASK + x];

  byte firstUnit = 1;
  byte lastUnit = UNIT_MAX - 1;
  if (destUnit != 0)
  {
    firstUnit = destUnit;
    lastUnit = destUnit;
  }
  for (byte x = firstUnit; x <= lastUnit; x++)
  {
    if (x != Settings.Unit){
      dataReply.destUnit = x;
      C013_sendUDP(x, (byte*) &dataReply, sizeof(dataStruct));
      delay(10);
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
  if (unit != 255)
    if (Nodes[unit].ip[0] == 0)
      return;
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
    remoteNodeIP = Nodes[unit].ip;
  C013_portUDP.beginPacket(remoteNodeIP, Settings.UDPPort);
  C013_portUDP.write(data, size);
  C013_portUDP.endPacket();
}

void C013_Receive(struct EventStruct *event) {
  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    if (event->Data[1] > 1 && event->Data[1] < 6)
    {
      String log = (F("C013 : msg "));
      for (byte x = 1; x < 6; x++)
      {
        log += " ";
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
        struct infoStruct infoReply;
        memcpy((byte*)&infoReply, (byte*)event->Data, sizeof(infoStruct));

        // to prevent flash wear out (bugs in communication?) we can only write to an empty task
        // so it will write only once and has to be cleared manually through webgui
        if (Settings.TaskDeviceNumber[infoReply.destTaskIndex] == 0)
        {
          Settings.TaskDeviceNumber[infoReply.destTaskIndex] = infoReply.deviceNumber;
          Settings.TaskDeviceDataFeed[infoReply.destTaskIndex] = 1;  // remote feed
          for (byte x = 0; x < CONTROLLER_MAX; x++)
            Settings.TaskDeviceSendData[x][infoReply.destTaskIndex] = false;
          strcpy(ExtraTaskSettings.TaskDeviceName, infoReply.taskName);
          for (byte x = 0; x < VARS_PER_TASK; x++)
            strcpy( ExtraTaskSettings.TaskDeviceValueNames[x], infoReply.ValueNames[x]);
          SaveTaskSettings(infoReply.destTaskIndex);
          SaveSettings();
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
        struct dataStruct dataReply;
        memcpy((byte*)&dataReply, (byte*)event->Data, sizeof(dataStruct));

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
        break;
      }
  }
}
#endif
