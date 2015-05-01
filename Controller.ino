//********************************************************************************
// Interface for Domoticz Controller, get and set data through json
//********************************************************************************

boolean Domoticz_getData(int idx, float *data)
{
  boolean success = false;
  char host[20];
  sprintf(host, "%u.%u.%u.%u", Settings.Server_IP[0], Settings.Server_IP[1], Settings.Server_IP[2], Settings.Server_IP[3]);

  Serial.print("HTTP : Connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, Settings.ServerPort)) {
    Serial.println("HTTP : Connection failed");
    return false;
  }

  // We now create a URI for the request
  String url = "/json.htm?type=devices&rid=";
  url += idx;

  Serial.print("HTTP : Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timer = millis() + 200;
  while (!client.available() && millis() < timer) {}

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
  Serial.println("HTTP : Closing connection");
  return success;
}

boolean Domoticz_sendData(int idx, float value)
{
  boolean success = false;
  char host[20];
  sprintf(host, "%u.%u.%u.%u", Settings.Server_IP[0], Settings.Server_IP[1], Settings.Server_IP[2], Settings.Server_IP[3]);

  Serial.print("HTTP : Connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, Settings.ServerPort)) {
    Serial.println("HTTP : Connection failed");
    return false;
  }

  // We now create a URI for the request
  String url = "/json.htm?type=command&param=udevice&idx=";
  url += idx;
  url += "&svalue=";
  url += value;

  Serial.print("HTTP : Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timer = millis() + 200;
  while (!client.available() && millis() < timer) {}

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line.substring(0, 15) == "HTTP/1.0 200 OK")
    {
      Serial.println("HTTP : Succes!");
      success = true;
    }
  }
  Serial.println("HTTP : Closing connection");
  return success;
}

