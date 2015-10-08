//********************************************************************************
// Interface for Sending to Controllers
//********************************************************************************
boolean sendData(struct EventStruct *event)
{
  LoadTaskSettings(event->TaskIndex);
  byte ProtocolIndex = getProtocolIndex(Settings.Protocol);
  CPlugin_ptr[ProtocolIndex](CPLUGIN_PROTOCOL_SEND, event);
      
  PluginCall(PLUGIN_EVENT_OUT, event, dummyString);
  
  if (Settings.MessageDelay != 0)
  {
    char log[30];
    sprintf_P(log, PSTR("HTTP : Delay %u ms"), Settings.MessageDelay);
    addLog(LOG_LEVEL_DEBUG_MORE, log);
    unsigned long timer = millis() + Settings.MessageDelay;
    while (millis() < timer)
      backgroundtasks();
  }
}


/*********************************************************************************************\
 * Handle incoming MQTT messages
\*********************************************************************************************/
// handle MQTT messages
void callback(const MQTT::Publish& pub) {
  char log[80];
  String message = pub.payload_string();
  String topic = pub.topic();

  char c_topic[80];
  topic.toCharArray(c_topic,80);
  sprintf_P(log, PSTR("%s%s"), "MQTT : Topic ", c_topic);
  addLog(LOG_LEVEL_DEBUG, log);

  struct EventStruct TempEvent;
  TempEvent.String1 = topic;
  TempEvent.String2 = message;
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
    if (MQTTclient.connect(clientid))
    {
      log = F("MQTT : Connected to broker");
      addLog(LOG_LEVEL_INFO,log);
      subscribeTo = Settings.MQTTsubscribe;
      subscribeTo.replace("%sysname%", Settings.Name);
      MQTTclient.subscribe(subscribeTo);
      log = F("Subscribed to: ");
      log += subscribeTo;
      addLog(LOG_LEVEL_INFO,log);
      break; // end loop if succesfull
    }
    else
      {
        log = F("MQTT : Failed to connected to broker");
        addLog(LOG_LEVEL_ERROR,log);
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
} Nodes[32];

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
void syslog(char *message)
{
  if (Settings.Syslog_IP[0] != 0)
  {
    IPAddress broadcastIP(Settings.Syslog_IP[0], Settings.Syslog_IP[1], Settings.Syslog_IP[2], Settings.Syslog_IP[3]);
    portTX.beginPacket(broadcastIP, 514);
    char str[80];
    str[0] = 0;
    sprintf_P(str, PSTR("<7>ESP Unit: %u : %s"), Settings.Unit, message);
    portTX.write(str);
    portTX.endPacket();
  }
}


/*********************************************************************************************\
 * Check UDP messages
\*********************************************************************************************/
void checkUDP()
{
  if (Settings.UDPPort == 0)
    return;

  // UDP events
  int packetSize = portRX.parsePacket();
  if (packetSize)
  {
    char packetBuffer[128];
    int len = portRX.read(packetBuffer, 128);
    if (packetBuffer[0] != 255)
    {
      packetBuffer[len] = 0;
      addLog(LOG_LEVEL_DEBUG, packetBuffer);
#ifdef ESP_CONNEXIO
      ExecuteLine(packetBuffer, VALUE_SOURCE_SERIAL);
#endif
#ifdef ESP_EASY
      ExecuteCommand(packetBuffer);
#endif
    }
    else
    {
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

            if (unit < 31)
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
          }
      }
    }
  }
}

void refreshNodeList()
{
  for (byte counter = 0; counter < 32; counter++)
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
    portTX.beginPacket(broadcastIP, Settings.UDPPort);
    portTX.write(data, 20);
    portTX.endPacket();
    if (counter < (repeats - 1))
      delay(500);
  }

  // store my own info also in the list...
  IPAddress ip = WiFi.localIP();
  for (byte x = 0; x < 4; x++)
    Nodes[Settings.Unit].ip[x] = ip[x];
  Nodes[Settings.Unit].age = 0;

}

