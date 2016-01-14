//********************************************************************************
// Interface for Sending to Controllers
//********************************************************************************
boolean sendData(struct EventStruct *event)
{

  if (!Settings.TaskDeviceSendData[event->TaskIndex])
    return false;

  if (Settings.MessageDelay != 0)
  {
    if ((millis() - lastSend) < Settings.MessageDelay)
    {
      char log[30];
      sprintf_P(log, PSTR("HTTP : Delay %u ms"), Settings.MessageDelay);
      addLog(LOG_LEVEL_DEBUG_MORE, log);
      unsigned long timer = millis() + Settings.MessageDelay;
      while (millis() < timer)
        backgroundtasks();
    }
  }

  if (Settings.TaskDeviceGlobalSync[event->TaskIndex])
    SendUDPTaskData(0, event->TaskIndex, event->TaskIndex);

  LoadTaskSettings(event->TaskIndex);
  if (Settings.Protocol)
  {
    byte ProtocolIndex = getProtocolIndex(Settings.Protocol);
    CPlugin_ptr[ProtocolIndex](CPLUGIN_PROTOCOL_SEND, event);
  }
  PluginCall(PLUGIN_EVENT_OUT, event, dummyString);
  lastSend = millis();
}


/*********************************************************************************************\
 * Handle incoming MQTT messages
\*********************************************************************************************/
// handle MQTT messages
void callback(const MQTT::Publish& pub) {
  char log[80];
  char tmp[80];
  String topic = pub.topic();
  String payload = pub.payload_string();

  topic.toCharArray(tmp, 80);
  sprintf_P(log, PSTR("%s%s"), "MQTT : Topic: ", tmp);
  addLog(LOG_LEVEL_DEBUG, log);
  payload.toCharArray(tmp, 80);
  sprintf_P(log, PSTR("%s%s"), "MQTT : Payload: ", tmp);
  addLog(LOG_LEVEL_DEBUG, log);

  struct EventStruct TempEvent;
  TempEvent.String1 = topic;
  TempEvent.String2 = payload;
  byte ProtocolIndex = getProtocolIndex(Settings.Protocol);
  CPlugin_ptr[ProtocolIndex](CPLUGIN_PROTOCOL_RECV, &TempEvent);
}


/*********************************************************************************************\
 * Connect to MQTT message broker
\*********************************************************************************************/
void MQTTConnect()
{
  IPAddress MQTTBrokerIP(Settings.Controller_IP);
  MQTTclient.set_server(MQTTBrokerIP, Settings.ControllerPort);
  MQTTclient.set_callback(callback);

  // MQTT needs a unique clientname to subscribe to broker
  String clientid = "ESPClient";
  clientid += Settings.Unit;
  String subscribeTo = "";

  for (byte x = 1; x < 3; x++)
  {
    String log = "";
    boolean MQTTresult = false;

    if ((SecuritySettings.ControllerUser) && (SecuritySettings.ControllerPassword))
      MQTTresult = (MQTTclient.connect(MQTT::Connect(clientid).set_auth(SecuritySettings.ControllerUser, SecuritySettings.ControllerPassword)));
    else
      MQTTresult = (MQTTclient.connect(clientid));

    if (MQTTresult)
    {
      log = F("MQTT : Connected to broker");
      addLog(LOG_LEVEL_INFO, log);
      subscribeTo = Settings.MQTTsubscribe;
      subscribeTo.replace("%sysname%", Settings.Name);
      MQTTclient.subscribe(subscribeTo);
      log = F("Subscribed to: ");
      log += subscribeTo;
      addLog(LOG_LEVEL_INFO, log);
      break; // end loop if succesfull
    }
    else
    {
      log = F("MQTT : Failed to connected to broker");
      addLog(LOG_LEVEL_ERROR, log);
    }

    delay(500);
  }
}


/*********************************************************************************************\
 * Check connection MQTT message broker
\*********************************************************************************************/
void MQTTCheck()
{
  byte ProtocolIndex = getProtocolIndex(Settings.Protocol);
  if (Protocol[ProtocolIndex].usesMQTT)
    if (!MQTTclient.connected())
    {
      MQTTclient.disconnect();
      delay(1000);
      MQTTConnect();
    }
}


struct NodeStruct
{
  byte ip[4];
  byte age;
} Nodes[UNIT_MAX];

/*********************************************************************************************\
 * Send data to other ESP node
\*********************************************************************************************/
boolean nodeVariableCopy(byte var, byte unit)
{
  float value = UserVar[var - 1];
  char log[80];
  boolean success = false;
  char host[20];

  if (Nodes[unit].ip[0] == 0)
  {
    strcpy_P(log, PSTR("Remote Node unknown"));
    addLog(LOG_LEVEL_DEBUG, log);
    if (printToWeb)
    {
      printWebString += log;
      printWebString += "<BR>";
    }
    return false;
  }

  sprintf_P(host, PSTR("%u.%u.%u.%u"), Nodes[unit].ip[0], Nodes[unit].ip[1], Nodes[unit].ip[2], Nodes[unit].ip[3]);

  sprintf_P(log, PSTR("%s%s"), "connecting to ", host);
  addLog(LOG_LEVEL_DEBUG, log);
  if (printToWeb)
  {
    printWebString += log;
    printWebString += "<BR>";
  }
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, 80))
  {
    connectionFailures++;
    strcpy_P(log, PSTR("HTTP : connection failed"));
    addLog(LOG_LEVEL_ERROR, log);
    if (printToWeb)
      printWebString += F("connection failed<BR>");
    return false;
  }
  if (connectionFailures)
    connectionFailures--;

  // We now create a URI for the request
  String url = F("/?cmd=variableset%20");
  url += var;
  url += ",";
  url += value;

  url.toCharArray(log, 80);
  addLog(LOG_LEVEL_DEBUG, log);
  if (printToWeb)
  {
    printWebString += log;
    printWebString += "<BR>";
  }

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timer = millis() + 200;
  while (!client.available() && millis() < timer)
    delay(1);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line.substring(0, 15) == "HTTP/1.1 200 OK")
    {
      strcpy_P(log, PSTR("HTTP : Succes!"));
      addLog(LOG_LEVEL_DEBUG, log);
      if (printToWeb)
        printWebString += F("Success<BR>");
      success = true;
    }
  }
  strcpy_P(log, PSTR("HTTP : closing connection"));
  addLog(LOG_LEVEL_DEBUG, log);
  if (printToWeb)
    printWebString += F("closing connection<BR>");

  return success;
}


/*********************************************************************************************\
 * Syslog client
\*********************************************************************************************/
void syslog(const char *message)
{
  if (Settings.Syslog_IP[0] != 0)
  {
    IPAddress broadcastIP(Settings.Syslog_IP[0], Settings.Syslog_IP[1], Settings.Syslog_IP[2], Settings.Syslog_IP[3]);
    portUDP.beginPacket(broadcastIP, 514);
    char str[80];
    str[0] = 0;
    sprintf_P(str, PSTR("<7>ESP Unit: %u : %s"), Settings.Unit, message);
    portUDP.write(str);
    portUDP.endPacket();
  }
}


/*********************************************************************************************\
 * Structs for UDP messaging
\*********************************************************************************************/
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

/*********************************************************************************************\
 * Check UDP messages
\*********************************************************************************************/
void checkUDP()
{
  if (Settings.UDPPort == 0)
    return;

  // UDP events
  int packetSize = portUDP.parsePacket();
  if (packetSize)
  {
    IPAddress remoteIP = portUDP.remoteIP();
    if (portUDP.remotePort() == 123)
    {
      // unexpected NTP reply, drop for now...
      return;
    }
    char packetBuffer[128];
    int len = portUDP.read(packetBuffer, 128);
    if (packetBuffer[0] != 255)
    {
      packetBuffer[len] = 0;
      addLog(LOG_LEVEL_DEBUG, packetBuffer);
      ExecuteCommand(packetBuffer);
    }
    else
    {
      if(packetBuffer[1] > 1 && packetBuffer[1] < 6)
      {
            Serial.println(F("UDP sensor message"));
            Serial.println((int)packetBuffer[1]); // unit
            Serial.println((int)packetBuffer[2]); // unit
            Serial.println((int)packetBuffer[3]); // unit
            Serial.println((int)packetBuffer[4]); // taskindex
            Serial.println((int)packetBuffer[5]); // taskindex
      }
      
      // binary data!
      switch (packetBuffer[1])
      {
        case 1: // sysinfo message
          {
            byte mac[6];
            byte ip[4];
            byte unit = packetBuffer[12];
            for (byte x = 0; x < 6; x++)
              mac[x] = packetBuffer[x + 2];
            for (byte x = 0; x < 4; x++)
              ip[x] = packetBuffer[x + 8];

            if (unit < UNIT_MAX)
            {
              for (byte x = 0; x < 4; x++)
                Nodes[unit].ip[x] = packetBuffer[x + 8];
              Nodes[unit].age = 0; // reset 'age counter'
            }

            char macaddress[20];
            sprintf_P(macaddress, PSTR("%02x:%02x:%02x:%02x:%02x:%02x"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            char ipaddress[16];
            sprintf_P(ipaddress, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
            char log[80];
            sprintf_P(log, PSTR("UDP  : %s,%s,%u"), macaddress, ipaddress, unit);
            addLog(LOG_LEVEL_DEBUG_MORE, log);
            break;
          }

        case 2: // sensor info pull request
          {
            SendUDPTaskInfo(packetBuffer[2], packetBuffer[5], packetBuffer[4]);
            break;
          }

        case 3: // sensor info
          {
            struct infoStruct infoReply;
            memcpy((byte*)&infoReply, (byte*)&packetBuffer, sizeof(infoStruct));

            // to prevent flash wear out (bugs in communication?) we can only write to an empty task
            // so it will write only once and has to be cleared manually through webgui
            if (Settings.TaskDeviceNumber[infoReply.destTaskIndex] == 0)
            {
              Settings.TaskDeviceNumber[infoReply.destTaskIndex] = infoReply.deviceNumber;
              Settings.TaskDeviceDataFeed[infoReply.destTaskIndex] = 1;  // remote feed
              Settings.TaskDeviceSendData[infoReply.destTaskIndex] = false;
              strcpy(ExtraTaskSettings.TaskDeviceName, infoReply.taskName);
              for (byte x = 0; x < VARS_PER_TASK; x++)
                strcpy( ExtraTaskSettings.TaskDeviceValueNames[x], infoReply.ValueNames[x]);
              SaveTaskSettings(infoReply.destTaskIndex);
              SaveSettings();
            }

            // display stuff for debugging
/*            Serial.println(infoReply.deviceNumber);
            String deviceName = "";
            byte DeviceIndex = getDeviceIndex(infoReply.deviceNumber);
            Plugin_ptr[DeviceIndex](PLUGIN_GET_DEVICENAME, 0, deviceName);
            Serial.println(deviceName);
            Serial.println(infoReply.taskName);
            for (byte x = 0; x < VARS_PER_TASK; x++)
              Serial.println(infoReply.ValueNames[x]);
*/              
            break;
          }

        case 4: // sensor data pull request
          {
            SendUDPTaskData(packetBuffer[2], packetBuffer[5], packetBuffer[4]);
            break;
          }

        case 5: // sensor data
          {
            struct dataStruct dataReply;
            memcpy((byte*)&dataReply, (byte*)&packetBuffer, sizeof(dataStruct));

            // only if this task has a remote feed, update values
            if (Settings.TaskDeviceDataFeed[dataReply.destTaskIndex] != 0)
            {
              for (byte x = 0; x < VARS_PER_TASK; x++)
              {
                UserVar[dataReply.destTaskIndex * VARS_PER_TASK + x] = dataReply.Values[x];
                //Serial.println(dataReply.Values[x]);
              }
            }
            break;
          }

        default:
          {
            struct EventStruct TempEvent;
            TempEvent.Data = (byte*)packetBuffer;
            TempEvent.Par1 = remoteIP[3];
            PluginCall(PLUGIN_UDP_IN, &TempEvent, dummyString);
            break;
          }
      }
    }
  }
}


/*********************************************************************************************\
 * Send task info using UDP message
\*********************************************************************************************/
void SendUDPTaskInfo(byte destUnit, byte sourceTaskIndex, byte destTaskIndex)
{
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
    sendUDP(x, (byte*)&infoReply, sizeof(infoStruct));
    delay(10);
  }
}


/*********************************************************************************************\
 * Send task data using UDP message
\*********************************************************************************************/
void SendUDPTaskData(byte destUnit, byte sourceTaskIndex, byte destTaskIndex)
{
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
    sendUDP(x, (byte*) &dataReply, sizeof(dataStruct));
    delay(10);
  }
}


/*********************************************************************************************\
 * Send UDP message
\*********************************************************************************************/
void sendUDP(byte unit, byte* data, byte size)
{
  if (Nodes[unit].ip[0] == 0)
    return;

  IPAddress remoteNodeIP(Nodes[unit].ip[0], Nodes[unit].ip[1], Nodes[unit].ip[2], Nodes[unit].ip[3]);
  portUDP.beginPacket(remoteNodeIP, Settings.UDPPort);
  portUDP.write(data, size);
  portUDP.endPacket();
}


/*********************************************************************************************\
 * Refresh aging for remote units, drop if too old...
\*********************************************************************************************/
void refreshNodeList()
{
  for (byte counter = 0; counter < UNIT_MAX; counter++)
  {
    if (Nodes[counter].ip[0] != 0)
    {
      Nodes[counter].age++;  // increment age counter
      if (Nodes[counter].age > 10) // if entry to old, clear this node ip from the list.
        for (byte x = 0; x < 4; x++)
          Nodes[counter].ip[x] = 0;
    }
  }
}

void sendSysInfoUDP(byte repeats)
{
  char log[80];
  if (Settings.UDPPort == 0)
    return;

  // 1 byte 'binary token 255'
  // 1 byte id
  // 6 byte mac
  // 4 byte ip
  // 1 byte unit

  // send my info to the world...
  strcpy_P(log, PSTR("UDP  : Send Sysinfo message"));
  addLog(LOG_LEVEL_DEBUG_MORE, log);
  for (byte counter = 0; counter < repeats; counter++)
  {
    uint8_t mac[] = {0, 0, 0, 0, 0, 0};
    uint8_t* macread = WiFi.macAddress(mac);
    byte data[20];
    data[0] = 255;
    data[1] = 1;
    for (byte x = 0; x < 6; x++)
      data[x + 2] = macread[x];
    IPAddress ip = WiFi.localIP();
    for (byte x = 0; x < 4; x++)
      data[x + 8] = ip[x];
    data[12] = Settings.Unit;

    IPAddress broadcastIP(255, 255, 255, 255);
    portUDP.beginPacket(broadcastIP, Settings.UDPPort);
    portUDP.write(data, 20);
    portUDP.endPacket();
    if (counter < (repeats - 1))
      delay(500);
  }

  // store my own info also in the list...
  IPAddress ip = WiFi.localIP();
  for (byte x = 0; x < 4; x++)
    Nodes[Settings.Unit].ip[x] = ip[x];
  Nodes[Settings.Unit].age = 0;

}

