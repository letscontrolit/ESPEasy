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

  LoadTaskSettings(event->TaskIndex);
  byte ProtocolIndex = getProtocolIndex(Settings.Protocol);
  CPlugin_ptr[ProtocolIndex](CPLUGIN_PROTOCOL_SEND, event);
  PluginCall(PLUGIN_EVENT_OUT, event, dummyString);
  lastSend = millis();
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
  topic.toCharArray(c_topic, 80);
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


time_t getNtpTime()
{
  WiFiUDP udp;
  udp.begin(123);
  String log="NTP  : NTP sync requested";
  addLog(LOG_LEVEL_DEBUG_MORE, log);

  const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
  byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

  IPAddress timeServerIP;
  const char* ntpServerName = "pool.ntp.org";

  if (Settings.NTPHost[0] !=0)
    WiFi.hostByName(Settings.NTPHost, timeServerIP);
  else
    WiFi.hostByName(ntpServerName, timeServerIP);

  char host[20];
  sprintf_P(host, PSTR("%u.%u.%u.%u"), timeServerIP[0], timeServerIP[1], timeServerIP[2], timeServerIP[3]);
  log="NTP  : NTP send to ";
  log += host;
  addLog(LOG_LEVEL_DEBUG_MORE, log);

  while (udp.parsePacket() > 0) ; // discard any previously received packets

  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  udp.beginPacket(timeServerIP, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();

  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      log="NTP  : NTP replied!";
      addLog(LOG_LEVEL_DEBUG_MORE, log);
      return secsSince1900 - 2208988800UL + Settings.TimeZone * SECS_PER_HOUR;
    }
  }
  log="NTP  : No reply";
  addLog(LOG_LEVEL_DEBUG_MORE, log);
  return 0;
}

