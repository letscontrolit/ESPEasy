//********************************************************************************
// Interface for Domoticz Controller, get and set data through json
//********************************************************************************

boolean Domoticz_getData(int idx, float *data)
{
  boolean success = false;
  char host[20];
  sprintf(host, "%u.%u.%u.%u", Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);

  Serial.print(F("HTTP : Connecting to "));
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, Settings.ControllerPort))
  {
    connectionFailures++;
    Serial.println(F("HTTP : Connection failed"));
    return false;
  }

  // We now create a URI for the request
  String url = "/json.htm?type=devices&rid=";
  url += idx;

  Serial.print(F("HTTP : Requesting URL: "));
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
      Serial.println("Succes!");
      success = true;
    }
  }
  Serial.println(F("HTTP : Closing connection"));
  return success;
}

boolean Domoticz_sendData(byte sensorType, int idx, byte varIndex)
{
  boolean success = false;
  char host[20];
  sprintf(host, "%u.%u.%u.%u", Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);

  Serial.print(F("HTTP : Connecting to "));
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, Settings.ControllerPort))
  {
    connectionFailures++;
    Serial.println(F("HTTP : Connection failed"));
    return false;
  }

  // We now create a URI for the request
  String url = F("/json.htm?type=command&param=udevice&idx=");
  url += idx;
  switch(sensorType)
  {
    case 1:                      // single value sensor, used for Dallas, BH1750, etc
      url += "&svalue=";
      url += UserVar[varIndex-1];
      break;
    case 2:                      // temp + hum + hum_stat, used for DHT11
      url += "&svalue=";
      url += UserVar[varIndex-1];
      url += ";";
      url += UserVar[varIndex];
      url += ";0";
      break;
    case 3:                      // temp + hum + hum_stat + bar + bar_fore, used for BMP085
      url += "&svalue=";
      url += UserVar[varIndex-1];
      url += ";0;0;";
      url += UserVar[varIndex];
      url += ";0";
      break;
  }
  
  Serial.print(F("HTTP : Requesting URL: "));
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
    if (line.substring(0, 15) == "HTTP/1.1 200 OK")
    {
      Serial.println("HTTP : Succes!");
      success = true;
    }
  }
  Serial.println(F("HTTP : Closing connection"));
  return success;
}

