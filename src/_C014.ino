#ifdef USES_C014
//#######################################################################################################
//########################### Controller Plugin 014: Blynk  #############################################
//#######################################################################################################

// #ifdef PLUGIN_BUILD_TESTING

#define CPLUGIN_014
#define CPLUGIN_ID_014         14
#define CPLUGIN_NAME_014       "Blynk"

#define _BLYNK_USE_DEFAULT_FREE_RAM
#define BLYNK_TIMEOUT_MS 2000UL
#define BLYNK_HEARTBEAT      30
#include <BlynkSimpleEsp8266.h>
// #include <BlynkSimpleEsp8266_SSL.h>

// called inside blynk connection process to keep espeasy stability
void CPlugin_014_handleInterrupt() {
  backgroundtasks();
}


bool CPlugin_014(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_014;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = false;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_014);
        break;
      }

    case CPLUGIN_INIT:
      {
       // when connected to another server and user has changed settings
       if (Blynk.connected()){
          addLog(LOG_LEVEL_INFO, F("BL: disconnect from server"));
          Blynk.disconnect();
       }
       break;
      }

     case CPLUGIN_PROTOCOL_SEND:
      {
        // Collect the values at the same run, to make sure all are from the same sample
        byte valueCount = getValueCountFromSensorType(event->sensorType);
        C014_queue_element element(event, valueCount);
        if (ExtraTaskSettings.TaskIndex != event->TaskIndex)
          PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummyString);

        for (byte x = 0; x < valueCount; x++)
        {
          bool isvalid;
          String formattedValue = formatUserVar(event, x, isvalid);
          if (!isvalid)
            formattedValue = F("error");

          String valueName = ExtraTaskSettings.TaskDeviceValueNames[x];
          String valueFullName = ExtraTaskSettings.TaskDeviceName;
          valueFullName += '.';
          valueFullName += valueName;
          String vPinNumberStr = valueName.substring(1, 4);
          int vPinNumber=vPinNumberStr.toInt();
          element.vPin[x] = vPinNumber;
          element.txt[x] = formattedValue;
          String log = F("BL ");
          log += Blynk.connected()? F("(online): ") : F("(offline): ");
          if (vPinNumber>0 && vPinNumber<256){
            log += "send ";
            log += valueFullName;
            log += " = ";
            log += formattedValue;
            log += " to blynk pin v";
            log += vPinNumber;
          }
          else{
            vPinNumber = -1;
            log += "error got vPin number for ";
            log += valueFullName;
            log += ", got not valid value: ";
            log += vPinNumberStr;
          }
          addLog(LOG_LEVEL_INFO, log);

        }
        success = C014_DelayHandler.addToQueue(element);
        scheduleNextDelayQueue(TIMER_C014_DELAY_QUEUE, C014_DelayHandler.getNextScheduleTime());
        break;
      }
  }
  return success;
}

//********************************************************************************
// Process Queued Blynk request, with data set to NULL
//********************************************************************************
// controller_plugin_number = 014 because of C014
bool do_process_c014_delay_queue(int controller_plugin_number, const C014_queue_element& element, ControllerSettingsStruct& ControllerSettings) {

  if (wifiStatus != ESPEASY_WIFI_SERVICES_INITIALIZED) {
    return false;
  }

  if (!Blynk_keep_connection_c014(element.controller_idx, ControllerSettings))
    return false;

  while (element.txt[element.valuesSent] == "" || element.vPin[element.valuesSent] == -1) {
  //   A non valid value, which we are not going to send.
  //   answer ok and skip real sending
    if (element.checkDone(true))
      return true;
  }

  return element.checkDone(Blynk_send_c014(element.txt[element.valuesSent], element.vPin[element.valuesSent]));
}


boolean Blynk_keep_connection_c014(int controllerIndex, ControllerSettingsStruct& ControllerSettings){
  if (!WiFiConnected())
    return false;

  if (!Blynk.connected()){
    String auth=SecuritySettings.ControllerPassword[controllerIndex];

    boolean connectDefault = false;
    String log = F("BL: ");

    static unsigned long blLastConnectAttempt = millis() - 60000;
    static String oldHost=ControllerSettings.getHost();

    if (ControllerSettings.getHost() != oldHost){
      log += "server has been changed, connect immideately";
      addLog(LOG_LEVEL_INFO, log);
      log = F("BL: ");
      oldHost=ControllerSettings.getHost();
      blLastConnectAttempt = millis() - 60000;
    }
    else{
      if (timePassedSince(blLastConnectAttempt)<60000){
        // log += "skip connect to blynk server too often. Wait a little...";
        // addLog(LOG_LEVEL_INFO, log);
        return false;
      }
      blLastConnectAttempt=millis();
    }


    if (ControllerSettings.UseDNS){
      String hostName = ControllerSettings.getHost();
      if (hostName.length() !=0){
        log += "Connecting to custom blynk server ";
        log += ControllerSettings.getHostPortString();
        Blynk.config(auth.c_str(),
                     CPlugin_014_handleInterrupt,
                     hostName.c_str(),
                     ControllerSettings.Port
                   );
      }
      else{
        log += "Custom blynk server name not specified. ";
        connectDefault = true;
      }
    }
    else{
      IPAddress ip = ControllerSettings.getIP();
      if ((ip[0] + ip[1] + ip[2] + ip[3])>0){
        log += "Connecting to custom blynk server ";
        log += ControllerSettings.getHostPortString();
        Blynk.config(auth.c_str(),
                     CPlugin_014_handleInterrupt,
                     ip,
                     ControllerSettings.Port
                   );
      }
      else{
        log += "Custom blynk server ip not specified. ";
        connectDefault = true;
      }
    }

    if (connectDefault){
      log += "Connecting go default server";
      Blynk.config(auth.c_str(),CPlugin_014_handleInterrupt);
    }

    addLog(LOG_LEVEL_INFO, log);
    Blynk.connect();
  }

  return Blynk.connected();
}


boolean Blynk_send_c014(const String& value, int vPin )
{
  Blynk.virtualWrite(vPin, value);
  // important - backgroundtasks - free mem
  unsigned long timer = millis() + Settings.MessageDelay;
  while (!timeOutReached(timer))
              backgroundtasks();
  return true;
}

// This is called for all virtual pins, that don't have BLYNK_WRITE handler
BLYNK_WRITE_DEFAULT() {
  byte vPin = request.pin;
  float pinValue = param.asFloat();
  String log = F("BL: server set v");
  log += vPin;
  log += F(" to ");
  log += pinValue;
  addLog(LOG_LEVEL_INFO, log);
  String eventCommand=F("blynkv");
  eventCommand += vPin;
  eventCommand += "=";
  eventCommand += pinValue;
  rulesProcessing(eventCommand);
}

BLYNK_CONNECTED() {
// Your code here when hardware connects to Blynk Cloud or private server.
// It’s common to call sync functions inside of this function.
// Requests all stored on the server latest values for all widgets.
  String eventCommand=F("blynk_connected");
  rulesProcessing(eventCommand);
  // addLog(LOG_LEVEL_INFO, F("BL: connected handler"));
}


// This is called when Smartphone App is opened
BLYNK_APP_CONNECTED() {
  String eventCommand=F("blynk_app_connected");
  rulesProcessing(eventCommand);
  // addLog(LOG_LEVEL_INFO, F("BL: app connected handler"));
}

// This is called when Smartphone App is closed
BLYNK_APP_DISCONNECTED() {
  String eventCommand=F("blynk_app_disconnected");
  rulesProcessing(eventCommand);
  // addLog(LOG_LEVEL_INFO, F("BL: app disconnected handler"));
}

#endif
