#ifndef COMMAND_HTTP_H
#define COMMAND_HTTP_H


bool Command_HTTP_SendToHTTP(struct EventStruct *event, const char* Line)
{
  if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED) {
      String strLine = Line;
      String host = parseString(strLine, 2);
      String port = parseString(strLine, 3);
      int pathpos = getParamStartPos(strLine, 4);
      String path = strLine.substring(pathpos);
      WiFiClient client;
      if (client.connect(host.c_str(), port.toInt()))
      {
        String reply = F("GET ");
        reply += path;
        reply += F(" HTTP/1.1\r\n");
        reply += F("Host: ");
        reply += host;
        reply += F("\r\n");
        reply += F("Connection: close\r\n\r\n");
        client.print(reply);

        unsigned long timer = millis() + 200;
        while (!client.available() && !timeOutReached(timer))
          delay(1);

        while (client.available()) {
          // String line = client.readStringUntil('\n');
          String line;
          safeReadStringUntil(client, line, '\n');


          if (line.substring(0, 15) == F("HTTP/1.1 200 OK"))
            addLog(LOG_LEVEL_DEBUG, line);
          delay(1);
        }
        client.flush();
        client.stop();
      }
    }
    return true;
}

#endif // COMMAND_HTTP_H