/*********************************************************************************************\
 * Get values from Domoticz using http/json
\*********************************************************************************************/
boolean Domoticz_getData(int idx, float *data)
{
  boolean success = false;
  char host[20];
  sprintf(host, "%u.%u.%u.%u", Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);

  Serial.print(F("connecting to "));
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, Settings.ControllerPort))
  {
    connectionFailures++;
    Serial.println(F("connection failed"));
    return false;
  }

  // We now create a URI for the request
  String url = F("/json.htm?type=devices&rid=");
  url += idx;

  Serial.print(F("Requesting URL: "));
  Serial.println(url);

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
    if (line.substring(10, 14) == "Data")
    {
      String strValue = line.substring(19);
      byte pos = strValue.indexOf(' ');
      strValue = strValue.substring(0, pos);
      strValue.trim();
      float value = strValue.toFloat();
      *data = value;
      Serial.println(F("Succes!"));
      success = true;
    }
  }
  Serial.println(F("closing connection"));
  return success;
}

//#ifdef ESP_EASY
//********************************************************************************
// Interface for Sending to Controllers
//********************************************************************************
boolean sendData(byte sensorType, int idx, byte varIndex)
{
  switch (Settings.Protocol)
  {
    case 1:
      Domoticz_sendData(sensorType, idx, varIndex);
      break;
    case 2:
      Domoticz_sendDataMQTT(sensorType, idx, varIndex);
      break;
  }
}
//#endif

/*********************************************************************************************\
 * Send data to Domoticz using http url querystring
\*********************************************************************************************/
boolean Domoticz_sendData(byte sensorType, int idx, byte varIndex)
{
  char log[80];
  boolean success = false;
  char host[20];
  sprintf(host, "%u.%u.%u.%u", Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);

  sprintf(log, "%s%s", "connecting to ", host);
  addLog(log);
  if (printToWeb)
  {
    printWebString += log;
    printWebString += "<BR>";
  }
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, Settings.ControllerPort))
  {
    connectionFailures++;
    addLog((char*)"connection failed");
    if (printToWeb)
      printWebString += F("connection failed<BR>");
    return false;
  }

  // We now create a URI for the request
  String url = F("/json.htm?type=command&param=udevice&idx=");
  url += idx;

  switch (sensorType)
  {
    case 1:                      // single value sensor, used for Dallas, BH1750, etc
      url += "&svalue=";
      url += UserVar[varIndex - 1];
      break;
    case 2:                      // temp + hum + hum_stat, used for DHT11
      url += "&svalue=";
      url += UserVar[varIndex - 1];
      url += ";";
      url += UserVar[varIndex];
      url += ";0";
      break;
    case 3:                      // temp + hum + hum_stat + bar + bar_fore, used for BMP085
      url += "&svalue=";
      url += UserVar[varIndex - 1];
      url += ";0;0;";
      url += UserVar[varIndex];
      url += ";0";
      break;
    case 10:                      // switch
      url = F("/json.htm?type=command&param=switchlight&idx=");
      url += idx;
      url += "&switchcmd=";
      if (UserVar[varIndex - 1] == 0)
        url += "Off";
      else
        url += "On";
      break;
  }

  url.toCharArray(log, 79);
  addLog(log);
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
      addLog((char*)"Succes!");
      if (printToWeb)
        printWebString += F("Success<BR>");
      success = true;
    }
  }
  addLog((char*)"closing connection");
  if (printToWeb)
    printWebString += F("closing connection<BR>");

#ifdef ESP_EASY
  if (Settings.UDPPort != 0)
  {
    IPAddress broadcastIP(255, 255, 255, 255);
    portTX.beginPacket(broadcastIP, Settings.UDPPort);
    String message = F("IDXEvent ");
    message += idx;
    message += ",";
    message += UserVar[varIndex - 1];
    char str[80];
    str[0] = 0;
    message.toCharArray(str, 79);
    Serial.print("UDP >: ");
    Serial.println(str);
    portTX.write(str);
    portTX.endPacket();
  }
#endif

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
    sprintf(str, "<7>ESP Unit: %u : %s", Settings.Unit, message);
    Serial.print("SYSLG > ");
    Serial.println(str);
    portTX.write(str);
    portTX.endPacket();
  }
}


/*********************************************************************************************\
 * Handle incoming MQTT messages
\*********************************************************************************************/
// handle MQTT messages
void callback(const MQTT::Publish& pub) {
  String message = pub.payload_string();
  MQTTMessage(&message);
}


/*********************************************************************************************\
 * Connect to MQTT message broker
\*********************************************************************************************/
void MQTTConnect()
{
  IPAddress MQTTBrokerIP(Settings.Controller_IP);
  MQTTclient.set_server(MQTTBrokerIP, 1883);
  MQTTclient.set_callback(callback);

  // MQTT needs a unique clientname to subscribe to broker
  String clientid = "ESPClient";
  clientid += Settings.Unit;

  for (byte x = 1; x < 3; x++)
  {
    if (MQTTclient.connect(clientid))
    {
      Serial.println(F("MQTT : Connected to broker"));
      MQTTclient.subscribe("domoticz/out");
      break;
    }
    else
      Serial.println(F("MQTT : Failed to connected to broker"));

    delay(500);
  }
}


/*********************************************************************************************\
 * Parse incoming MQTT message
\*********************************************************************************************/
void MQTTMessage(String *message)
{
  char json[512];
  json[0] = 0;
  message->toCharArray(json, 511);
  //  if (Settings.Debug == 3)
  //    Serial.println(json);

  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);

  //for (JsonObject::iterator it=root.begin(); it!=root.end(); ++it)
  //{
  //  Serial.println(it->key);
  //  Serial.println(it->value.asString());
  //}

  if (root.success())
  {
    long idx = root["idx"];
    float nvalue = root["nvalue"];
    long nvaluealt = root["nvalue"];
    const char* name = root["name"];
    const char* svalue = root["svalue"];
    const char* svalue1 = root["svalue1"];
    const char* svalue2 = root["svalue2"];
    const char* svalue3 = root["svalue3"];

    if (nvalue == 0)
      nvalue = nvaluealt;

    Serial.print(F("MQTT : idx="));
    Serial.print(idx);
    Serial.print(" name=");
    Serial.print(name);
    Serial.print(" nvalue=");
    Serial.print(nvalue);
    Serial.print(" svalue=");
    Serial.print(svalue);
    Serial.print(" svalue1=");
    Serial.print(svalue1);
    Serial.print(" svalue2=");
    Serial.println(svalue2);
    Serial.print(" svalue3=");
    Serial.println(svalue3);
  }
  else
    Serial.println(F("MQTT : json parse error"));
}


/*********************************************************************************************\
 * Send data to Domoticz using MQTT message
\*********************************************************************************************/
boolean Domoticz_sendDataMQTT(byte sensorType, int idx, byte varIndex)
{
  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();

  root["idx"] = idx;

  String values;
  char str[80];

  switch (sensorType)
  {
    case 1:                      // single value sensor, used for Dallas, BH1750, etc
      root["nvalue"] = 0;
      values = UserVar[varIndex - 1];
      values.toCharArray(str, 79);
      root["svalue"] =  str;
      break;
    case 2:                      // temp + hum + hum_stat, used for DHT11
      root["nvalue"] = 0;
      values  = UserVar[varIndex - 1];
      values += ";";
      values += UserVar[varIndex];
      values += ";0";
      values.toCharArray(str, 79);
      root["svalue"] =  str;
      break;
    case 3:                      // temp + hum + hum_stat + bar + bar_fore, used for BMP085
      root["nvalue"] = 0;
      values  = UserVar[varIndex - 1];
      values += ";0;0;";
      values += UserVar[varIndex];
      values += ";0";
      values.toCharArray(str, 79);
      root["svalue"] =  str;
      break;
    case 10:                      // switch
      root["command"] = "switchlight";
      if (UserVar[varIndex - 1] == 0)
        root["switchcmd"] = "Off";
      else
        root["switchcmd"] = "On";
      break;
  }

  //  JsonArray& data = root.createNestedArray("data");
  //  data.add(48.756080, 6);  // 6 is the number of decimals to print
  //  data.add(2.302038, 6);   // if not specified, 2 digits are printed

  // test drive mqtt
  //String json = "{\"idx\": 161, \"nvalue\": 1, \"svalue\": \"";
  //json += millis()/1000;
  //json += "\" }";
  //Serial.println(json);

  char json[256];
  root.printTo(json, sizeof(json));
  Serial.print("MQTT : ");
  Serial.println(json);
  addLog(json);
  if (!MQTTclient.publish("domoticz/in", json))
  {
    Serial.println(F("MQTT publish failed"));
    MQTTConnect();
    connectionFailures++;
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
    strcpy(log, "Remote Node unknown");
    addLog(log);
    if (printToWeb)
    {
      printWebString += log;
      printWebString += "<BR>";
    }
    return false;
  }

  sprintf(host, "%u.%u.%u.%u", Nodes[unit].ip[0], Nodes[unit].ip[1], Nodes[unit].ip[2], Nodes[unit].ip[3]);

  sprintf(log, "%s%s", "connecting to ", host);
  addLog(log);
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
    addLog((char*)"connection failed");
    if (printToWeb)
      printWebString += F("connection failed<BR>");
    return false;
  }

  // We now create a URI for the request
  String url = F("/?cmd=variableset%20");
  url += var;
  url += ",";
  url += value;

  url.toCharArray(log, 79);
  addLog(log);
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
      addLog((char*)"Succes!");
      if (printToWeb)
        printWebString += F("Success<BR>");
      success = true;
    }
  }
  addLog((char*)"closing connection");
  if (printToWeb)
    printWebString += F("closing connection<BR>");

  return success;
}


/*********************************************************************************************\
 * Check UDP messages
\*********************************************************************************************/
void checkUDP()
{
  // UDP events
  int packetSize = portRX.parsePacket();
  if (packetSize)
  {
    char packetBuffer[128];
    Serial.print("UDP " );
    Serial.print(packetSize);
    Serial.print(" < ");
    int len = portRX.read(packetBuffer, 128);
    if (packetBuffer[0] != 255)
    {
      packetBuffer[len] = 0;
      Serial.println(packetBuffer);
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
            sprintf(macaddress, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            char ipaddress[16];
            sprintf(ipaddress, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
            Serial.print(macaddress);
            Serial.print(",");
            Serial.print(ipaddress);
            Serial.print(",");
            Serial.println(unit);
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
  // 1 byte 'binary token 255'
  // 1 byte id
  // 6 byte mac
  // 4 byte ip
  // 1 byte unit
  // ??? build
  // ??
  // send my info to the world...
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

