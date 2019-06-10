#ifdef USES_C003
//#######################################################################################################
//########################### Controller Plugin 003: Nodo Telnet  #######################################
//#######################################################################################################

#define CPLUGIN_003
#define CPLUGIN_ID_003         3
#define CPLUGIN_NAME_003       "Nodo Telnet"

bool CPlugin_003(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_003;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 23;
        Protocol[protocolCount].usesID = true;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_003);
        break;
      }

    case CPLUGIN_INIT:
      {
        MakeControllerSettings(ControllerSettings);
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);
        C003_DelayHandler.configureControllerSettings(ControllerSettings);
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        // We now create a URI for the request
        String url = F("variableset ");
        url += event->idx;
        url += ",";
        url += formatUserVarNoCheck(event, 0);
        url += "\n";
        success = C003_DelayHandler.addToQueue(C003_queue_element(event->ControllerIndex, url));
        scheduleNextDelayQueue(TIMER_C003_DELAY_QUEUE, C003_DelayHandler.getNextScheduleTime());

        break;
      }

  }
  return success;
}

bool do_process_c003_delay_queue(int controller_number, const C003_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  bool success = false;
  char log[80];
  addLog(LOG_LEVEL_DEBUG, String(F("TELNT : connecting to ")) + ControllerSettings.getHostPortString());
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!ControllerSettings.connectToHost(client))
  {
    connectionFailures++;
    strcpy_P(log, PSTR("TELNT: connection failed"));
    addLog(LOG_LEVEL_ERROR, log);
    return success;
  }
  statusLED(true);
  if (connectionFailures)
    connectionFailures--;

  // strcpy_P(log, PSTR("TELNT: Sending enter"));
  // addLog(LOG_LEVEL_ERROR, log);
  client.print(" \n");

  unsigned long timer = millis() + 200;
  while (!client_available(client) && !timeOutReached(timer))
    delay(1);

  timer = millis() + 1000;
  while (client_available(client) && !timeOutReached(timer) && !success)
  {

    //   String line = client.readStringUntil('\n');
    String line;
    safeReadStringUntil(client, line, '\n');

    if (line.startsWith(F("Enter your password:")))
    {
      success = true;
      strcpy_P(log, PSTR("TELNT: Password request ok"));
      addLog(LOG_LEVEL_DEBUG, log);
    }
    delay(1);
  }

  strcpy_P(log, PSTR("TELNT: Sending pw"));
  addLog(LOG_LEVEL_DEBUG, log);
  client.println(SecuritySettings.ControllerPassword[element.controller_idx]);
  delay(100);
  while (client_available(client))
    client.read();

  strcpy_P(log, PSTR("TELNT: Sending cmd"));
  addLog(LOG_LEVEL_DEBUG, log);
  client.print(element.txt);
  delay(10);
  while (client_available(client))
    client.read();

  strcpy_P(log, PSTR("TELNT: closing connection"));
  addLog(LOG_LEVEL_DEBUG, log);

  client.stop();
  return success;
}
#endif
