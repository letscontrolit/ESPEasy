/*********************************************************************************************\
 * Get values from Domoticz using http/json
\*********************************************************************************************/
boolean Domoticz_getData(int idx, float *data)
{
  boolean success = false;
  char host[20];
  sprintf_P(host, PSTR("%u.%u.%u.%u"), Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, Settings.ControllerPort))
  {
    connectionFailures++;
    return false;
  }
  if (connectionFailures)
    connectionFailures--;

  // We now create a URI for the request
  String url = F("/json.htm?type=devices&rid=");
  url += idx;

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
  return success;
}

//#ifdef ESP_EASY
//********************************************************************************
// Interface for Sending to Controllers
//********************************************************************************
boolean sendData(struct EventStruct *event)
{
  LoadTaskSettings(event->TaskIndex);
  switch (Settings.Protocol)
  {
    case PROTOCOL_DOMOTICZ_HTTP:
      Domoticz_sendData(event);
      break;
    case PROTOCOL_DOMOTICZ_MQTT:
      Domoticz_sendDataMQTT(event);
      break;
    case PROTOCOL_NODO_TELNET:
      NodoTelnet_sendData(event);
    case PROTOCOL_THINGSPEAK:
      ThingsSpeak_sendData(event);
      break;
    case PROTOCOL_OPENHAB_MQTT:
      OpenHAB_sendDataMQTT(event);
      break;
    case PROTOCOL_PIDOME_MQTT:
      PiDome_sendDataMQTT(event);
      break;
  }

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
//#endif

/*********************************************************************************************\
 * Send data to Domoticz using http url querystring
\*********************************************************************************************/
boolean Domoticz_sendData(struct EventStruct *event)
{
  char log[80];
  boolean success = false;
  char host[20];
  sprintf_P(host, PSTR("%u.%u.%u.%u"), Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);

  sprintf_P(log, PSTR("%s%s"), "HTTP : connecting to ", host);
  addLog(LOG_LEVEL_DEBUG, log);
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
    strcpy_P(log, PSTR("HTTP : connection failed"));
    addLog(LOG_LEVEL_ERROR, log);
    if (printToWeb)
      printWebString += F("connection failed<BR>");
    return false;
  }
  if (connectionFailures)
    connectionFailures--;

  // We now create a URI for the request
  String url = F("/json.htm?type=command&param=udevice&idx=");
  url += event->idx;

  switch (event->sensorType)
  {
    case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
      url += F("&svalue=");
      url += UserVar[event->BaseVarIndex];
      break;
    case SENSOR_TYPE_TEMP_HUM:                      // temp + hum + hum_stat, used for DHT11
      url += F("&svalue=");
      url += UserVar[event->BaseVarIndex];
      url += ";";
      url += UserVar[event->BaseVarIndex + 1];
      url += ";0";
      break;
    case SENSOR_TYPE_TEMP_BARO:                      // temp + hum + hum_stat + bar + bar_fore, used for BMP085
      url += F("&svalue=");
      url += UserVar[event->BaseVarIndex];
      url += ";0;0;";
      url += UserVar[event->BaseVarIndex + 1];
      url += ";0";
      break;
    case SENSOR_TYPE_SWITCH:
      url = F("/json.htm?type=command&param=switchlight&idx=");
      url += event->idx;
      url += F("&switchcmd=");
      if (UserVar[event->BaseVarIndex] == 0)
        url += "Off";
      else
        url += "On";
      break;
    case SENSOR_TYPE_DIMMER:
      url = F("/json.htm?type=command&param=switchlight&idx=");
      url += event->idx;
      url += F("&switchcmd=");
      if (UserVar[event->BaseVarIndex] == 0)
        url += "Off";
      else
      {
        url += F("Set%20Level&level=");
        url += UserVar[event->BaseVarIndex];
      }
      break;
  }

  url.toCharArray(log, 80);
  addLog(LOG_LEVEL_DEBUG_MORE, log);
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
    line.toCharArray(log, 80);
    addLog(LOG_LEVEL_DEBUG_MORE, log);
    if (line.substring(0, 15) == "HTTP/1.1 200 OK")
    {
      strcpy_P(log, PSTR("HTTP : Succes!"));
      addLog(LOG_LEVEL_DEBUG, log);
      if (printToWeb)
        printWebString += F("Success<BR>");
      success = true;
    }
    delay(1);
  }
  strcpy_P(log, PSTR("HTTP : closing connection"));
  addLog(LOG_LEVEL_DEBUG, log);
  if (printToWeb)
    printWebString += F("closing connection<BR>");

  client.flush();
  client.stop();
  return success;
}


/*********************************************************************************************\
 * Send data to Domoticz using http url querystring
\*********************************************************************************************/
boolean NodoTelnet_sendData(struct EventStruct *event)
{
  char log[80];
  boolean success = false;
  char host[20];
  sprintf_P(host, PSTR("%u.%u.%u.%u"), Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);

  sprintf_P(log, PSTR("%s%s"), "TELNT: connecting to ", host);
  addLog(LOG_LEVEL_DEBUG, log);
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
    strcpy_P(log, PSTR("TELNT: connection failed"));
    addLog(LOG_LEVEL_ERROR, log);
    if (printToWeb)
      printWebString += F("connection failed<BR>");
    return false;
  }
  if (connectionFailures)
    connectionFailures--;

  float value = UserVar[event->BaseVarIndex];
  // We now create a URI for the request
  String url = F("variableset ");
  url += event->idx;
  url += ",";
  url += value;
  url += "\n";

  Serial.println(F("Sending enter"));
  client.print(" \n");

  unsigned long timer = millis() + 200;
  while (!client.available() && millis() < timer)
    delay(1);

  timer = millis() + 1000;
  while (client.available() && millis() < timer && !success)
  {
    String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line.substring(0, 20) == "Enter your password:")
    {
      success = true;
      Serial.println(F("Password request ok"));
    }
    delay(1);
  }

  Serial.println(F("Sending pw"));
  client.println(SecuritySettings.ControllerPassword);
  delay(100);
  while (client.available())
    Serial.write(client.read());

  Serial.println(F("Sending cmd"));
  client.print(url);
  delay(10);
  while (client.available())
    Serial.write(client.read());

  strcpy_P(log, PSTR("TELNT: closing connection"));
  addLog(LOG_LEVEL_DEBUG, log);
  if (printToWeb)
    printWebString += F("closing connection<BR>");

  client.stop();
  return success;
}


/*********************************************************************************************\
 * Handle incoming MQTT messages
\*********************************************************************************************/
// handle MQTT messages
void callback(const MQTT::Publish& pub) {
  String message = pub.payload_string();
  String topic = pub.topic();
  MQTTMessage(&topic, &message);
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
    if (MQTTclient.connect(clientid))
    {
      Serial.println(F("MQTT : Connected to broker"));
      subscribeTo = Settings.MQTTsubscribe;
      subscribeTo.replace("%sysname%", Settings.Name);
      MQTTclient.subscribe(subscribeTo);
      Serial.print(F("Subscribed to: "));
      Serial.println(subscribeTo);
      break; // end loop if succesfull
    }
    else
      Serial.println(F("MQTT : Failed to connected to broker"));

    delay(500);
  }
}


/*********************************************************************************************\
 * Check connection MQTT message broker
\*********************************************************************************************/
void MQTTCheck()
{
  if ((Settings.Protocol == PROTOCOL_DOMOTICZ_MQTT) || (Settings.Protocol == PROTOCOL_OPENHAB_MQTT) || (Settings.Protocol == PROTOCOL_PIDOME_MQTT))
  if (!MQTTclient.connected())
  {
    MQTTclient.disconnect();
    delay(1000);
    MQTTConnect();
  }
}


/*********************************************************************************************\
 * Parse incoming MQTT message
\*********************************************************************************************/
void MQTTMessage(String *topic, String *message)
{
  char log[80];
  char c_topic[80];
  topic->toCharArray(c_topic,80);
  sprintf_P(log, PSTR("%s%s"), "MQTT : Topic ", c_topic);
  addLog(LOG_LEVEL_DEBUG, log);
  
  // Split topic into array
  String tmpTopic = topic->substring(1);
  String topicSplit[10];
  int SlashIndex = tmpTopic.indexOf('/');
  byte count = 0;
  while (SlashIndex > 0 && count < 10 - 1)
  {
    topicSplit[count] = tmpTopic.substring(0, SlashIndex);
    tmpTopic = tmpTopic.substring(SlashIndex + 1);
    SlashIndex = tmpTopic.indexOf('/');
    count++;
  }
  topicSplit[count] = tmpTopic;

  switch (Settings.Protocol)
  {
    case PROTOCOL_DOMOTICZ_MQTT:
      {
        char json[512];
        json[0] = 0;
        message->toCharArray(json, 512);

        StaticJsonBuffer<512> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(json);

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
          Serial.print(F(" name="));
          Serial.print(name);
          Serial.print(F(" nvalue="));
          Serial.print(nvalue);
          Serial.print(F(" svalue="));
          Serial.print(svalue);
          Serial.print(F(" svalue1="));
          Serial.print(svalue1);
          Serial.print(F(" svalue2="));
          Serial.println(svalue2);
          Serial.print(F(" svalue3="));
          Serial.println(svalue3);
        }
        else
          Serial.println(F("MQTT : json parse error"));
        break;
      }
    case PROTOCOL_OPENHAB_MQTT:
      {
        String cmd = topicSplit[1];
        int pin = topicSplit[2].toInt();
        int value = message->toFloat();
        struct EventStruct TempEvent;
        TempEvent.Par1 = pin;
        TempEvent.Par2 = value;
        PluginCall(PLUGIN_WRITE, &TempEvent, cmd);
        break;
      }

    case PROTOCOL_PIDOME_MQTT:
      {
        String name = topicSplit[4];
        String cmd = topicSplit[5];
        int pin = topicSplit[6].toInt();
        int value = (*message == "true");
        struct EventStruct TempEvent;
        TempEvent.Par1 = pin;
        TempEvent.Par2 = value;
        if (name == Settings.Name)
        {
          PluginCall(PLUGIN_WRITE, &TempEvent, cmd);
        }
        break;
      }
  }
}


/*********************************************************************************************\
 * Send data to Domoticz using MQTT message
\*********************************************************************************************/
boolean Domoticz_sendDataMQTT(struct EventStruct *event)
{
  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();

  root["idx"] = event->idx;

  String values;
  char str[80];

  switch (event->sensorType)
  {
    case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
      root["nvalue"] = 0;
      values = UserVar[event->BaseVarIndex];
      values.toCharArray(str, 80);
      root["svalue"] =  str;
      break;
    case SENSOR_TYPE_TEMP_HUM:                      // temp + hum + hum_stat, used for DHT11
      root["nvalue"] = 0;
      values  = UserVar[event->BaseVarIndex];
      values += ";";
      values += UserVar[event->BaseVarIndex + 1];
      values += ";0";
      values.toCharArray(str, 80);
      root["svalue"] =  str;
      break;
    case SENSOR_TYPE_TEMP_BARO:                      // temp + hum + hum_stat + bar + bar_fore, used for BMP085
      root["nvalue"] = 0;
      values  = UserVar[event->BaseVarIndex];
      values += ";0;0;";
      values += UserVar[event->BaseVarIndex + 1];
      values += ";0";
      values.toCharArray(str, 80);
      root["svalue"] =  str;
      break;
    case SENSOR_TYPE_SWITCH:
      root["command"] = "switchlight";
      if (UserVar[event->BaseVarIndex] == 0)
        root["switchcmd"] = "Off";
      else
        root["switchcmd"] = "On";
      break;
    case SENSOR_TYPE_DIMMER:
      root["command"] = "switchlight";
      if (UserVar[event->BaseVarIndex] == 0)
        root["switchcmd"] = "Off";
      else
        root["Set%20Level"] = UserVar[event->BaseVarIndex];
      break;
  }

  char json[256];
  root.printTo(json, sizeof(json));
  Serial.print("MQTT : ");
  Serial.println(json);
  addLog(LOG_LEVEL_DEBUG, json);

  String pubname = Settings.MQTTpublish;
  pubname.replace("%sysname%", Settings.Name);
  pubname.replace("%tskname%", ExtraTaskSettings.TaskDeviceName);
  pubname.replace("%id%", String(event->idx));
  
  if (!MQTTclient.publish(pubname, json))
  {
    Serial.println(F("MQTT publish failed"));
    MQTTConnect();
    connectionFailures++;
  }
  else if (connectionFailures)
    connectionFailures--;
}


/*********************************************************************************************\
 * Send data to Domoticz using MQTT message
\*********************************************************************************************/
boolean OpenHAB_sendDataMQTT(struct EventStruct *event)
{
  // MQTT publish structure:
  // /<unit name>/<task name>/<value name>

  char str[80];
  if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0)
    PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummyString);

  String pubname = Settings.MQTTpublish;
  pubname.replace("%sysname%", Settings.Name);
  pubname.replace("%tskname%", ExtraTaskSettings.TaskDeviceName);
  pubname.replace("%id%", String(event->idx));

  String value = "";
  byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[event->TaskIndex]);

  switch (event->sensorType)
  {
    case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
    case SENSOR_TYPE_SWITCH:
    case SENSOR_TYPE_DIMMER:
      pubname.replace("%valname%", ExtraTaskSettings.TaskDeviceValueNames[0]);
      value = String(UserVar[event->BaseVarIndex]);
      Serial.print(pubname);
      Serial.print(":");
      Serial.println(value);
      MQTTclient.publish(pubname, value);
      break;
    case SENSOR_TYPE_TEMP_HUM:
    case SENSOR_TYPE_TEMP_BARO:
      String tmppubname = pubname;
      tmppubname.replace("%valname%", ExtraTaskSettings.TaskDeviceValueNames[0]);
      value = String(UserVar[event->BaseVarIndex]);
      MQTTclient.publish(tmppubname, value);
      tmppubname = pubname;
      tmppubname.replace("%valname%", ExtraTaskSettings.TaskDeviceValueNames[1]);
      value = String(UserVar[event->BaseVarIndex + 1]);
      MQTTclient.publish(tmppubname, value);
      break;
  }
}


/*********************************************************************************************\
 * Send data to Domoticz using MQTT message
\*********************************************************************************************/
boolean PiDome_sendDataMQTT(struct EventStruct *event)
{
  // MQTT publish structure:
  // /hooks/devices/idx/groupid/value name

  char str[80];
  if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0)
    PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummyString);

  String pubname = Settings.MQTTpublish;
  pubname.replace("%sysname%", Settings.Name);
  pubname.replace("%tskname%", ExtraTaskSettings.TaskDeviceName);
  pubname.replace("%id%", String(event->idx));

  String value = "";
  byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[event->TaskIndex]);

  switch (event->sensorType)
  {
    case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
    case SENSOR_TYPE_SWITCH:
    case SENSOR_TYPE_DIMMER:
      pubname.replace("%valname%", ExtraTaskSettings.TaskDeviceValueNames[0]);
      value = String(UserVar[event->BaseVarIndex]);
      Serial.print(pubname);
      Serial.print(":");
      Serial.println(value);
      MQTTclient.publish(pubname, value);
      break;
    case SENSOR_TYPE_TEMP_HUM:
    case SENSOR_TYPE_TEMP_BARO:
      String tmppubname = pubname;
      tmppubname.replace("%valname%", ExtraTaskSettings.TaskDeviceValueNames[0]);
      value = String(UserVar[event->BaseVarIndex]);
      MQTTclient.publish(tmppubname, value);
      tmppubname = pubname;
      tmppubname.replace("%valname%", ExtraTaskSettings.TaskDeviceValueNames[1]);
      value = String(UserVar[event->BaseVarIndex + 1]);
      MQTTclient.publish(tmppubname, value);
      break;
  }
}


/*********************************************************************************************\
 * Send data to Domoticz using http url querystring
\*********************************************************************************************/
boolean ThingsSpeak_sendData(struct EventStruct *event)
{
  char log[80];
  boolean success = false;
  char host[20];
  sprintf_P(host, PSTR("%u.%u.%u.%u"), Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);

  sprintf_P(log, PSTR("%s%s"), "HTTP : connecting to ", host);
  addLog(LOG_LEVEL_DEBUG, log);
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
    strcpy_P(log, PSTR("HTTP : connection failed"));
    addLog(LOG_LEVEL_ERROR, log);
    if (printToWeb)
      printWebString += F("connection failed<BR>");
    return false;
  }
  if (connectionFailures)
    connectionFailures--;

  String postDataStr = SecuritySettings.ControllerPassword; // "0UDNN17RW6XAS2E5" // api key

  switch (event->sensorType)
  {
    case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
      postDataStr += F("&field");
      postDataStr += event->idx;
      postDataStr += "=";
      postDataStr += String(UserVar[event->BaseVarIndex]);
      break;
    case SENSOR_TYPE_TEMP_HUM:                      // dual value
    case SENSOR_TYPE_TEMP_BARO:
      postDataStr += F("&field");
      postDataStr += event->idx;
      postDataStr += "=";
      postDataStr += String(UserVar[event->BaseVarIndex]);
      postDataStr += F("&field");
      postDataStr += event->idx + 1;
      postDataStr += "=";
      postDataStr += String(UserVar[event->BaseVarIndex + 1]);
      break;
    case SENSOR_TYPE_SWITCH:
      break;
  }
  postDataStr += F("\r\n\r\n");

  String postStr = F("POST /update HTTP/1.1\n");
  postStr += F("Host: api.thingspeak.com\n");
  postStr += F("Connection: close\n");
  postStr += F("X-THINGSPEAKAPIKEY: ");
  postStr += SecuritySettings.ControllerPassword;
  postStr += "\n";
  postStr += F("Content-Type: application/x-www-form-urlencoded\n");
  postStr += F("Content-Length: ");
  postStr += postDataStr.length();
  postStr += F("\n\n");
  postStr += postDataStr;

  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)
    Serial.println(postStr);

  // This will send the request to the server
  client.print(postStr);

  unsigned long timer = millis() + 200;
  while (!client.available() && millis() < timer)
    delay(1);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)
    {
      sprintf_P(log, PSTR("C1 WS %u FM %u"), WiFi.status(), FreeMem());
      Serial.println(log);
    }
    String line = client.readStringUntil('\n');
    if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)
    {
      sprintf_P(log, PSTR("C2 WS %u FM %u"), WiFi.status(), FreeMem());
      Serial.println(log);
    }
    line.toCharArray(log, 80);
    addLog(LOG_LEVEL_DEBUG_MORE, log);
    if (line.substring(0, 15) == "HTTP/1.1 200 OK")
    {
      strcpy_P(log, PSTR("HTTP : Succes!"));
      addLog(LOG_LEVEL_DEBUG, log);
      if (printToWeb)
        printWebString += F("Success<BR>");
      success = true;
    }
    delay(1);
  }
  strcpy_P(log, PSTR("HTTP : closing connection"));
  addLog(LOG_LEVEL_DEBUG, log);
  if (printToWeb)
    printWebString += F("closing connection<BR>");

  client.flush();
  client.stop();
  return success;
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

