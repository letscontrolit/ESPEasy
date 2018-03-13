//#######################################################################################################
//########################### Controller Plugin 013: ESPEasy P2P network ################################
//#######################################################################################################

#define CPLUGIN_013
#define CPLUGIN_ID_013         13
#define CPLUGIN_NAME_013       "ESPEasy P2P Networking"

WiFiUDP C013_portUDP;

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
        Serial.println("C013 : Init");
        C013_portUDP.begin(Settings.UDPPort);
        break;
      }
      
      case CPLUGIN_TASK_CHANGE_NOTIFICATION:
      {
        Serial.println("C013 : Send task config");
        C013_SendUDPTaskInfo(0, event->TaskIndex, event->TaskIndex);
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        C013_Send(event, 0, UserVar[event->BaseVarIndex], 0);
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
    infoReply.destUnit = x;
    C013_sendUDP(x, (byte*)&infoReply, sizeof(infoStruct));
    delay(10);
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
    dataReply.destUnit = x;
    C013_sendUDP(x, (byte*) &dataReply, sizeof(dataStruct));
    delay(10);
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
  String log = "C013 : Send UDP message to ";
  log += unit;
  addLog(LOG_LEVEL_DEBUG_MORE, log);

  statusLED(true);

  IPAddress remoteNodeIP;
  if (unit == 255)
    remoteNodeIP = {255,255,255,255};
  else
    remoteNodeIP = Nodes[unit].ip;
  C013_portUDP.beginPacket(remoteNodeIP, Settings.UDPPort);
  C013_portUDP.write(data, size);
  C013_portUDP.endPacket();
}

