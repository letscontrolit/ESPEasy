#ifdef USES_C014
//#######################################################################################################
//########################### Controller Plugin 014: Blynk  #############################################
//#######################################################################################################

// #ifdef PLUGIN_BUILD_TESTING

#define CPLUGIN_014
#define CPLUGIN_ID_014         14
#define CPLUGIN_NAME_014       "Blynk"

#define BLYNK_TIMEOUT_MS 2000UL
#define BLYNK_HEARTBEAT      30
#include <BlynkSimpleEsp8266.h>
// #include <BlynkSimpleEsp8266_SSL.h>


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

     case CPLUGIN_PROTOCOL_SEND:
      {

        if (WiFiConnected())
          if (!Blynk.connected()){
            String auth=SecuritySettings.ControllerPassword[event->ControllerIndex];

            MakeControllerSettings(ControllerSettings);
            LoadControllerSettings(event->ControllerIndex, ControllerSettings);
            boolean connectDefault = false;
            String log = F("BL: ");

            static unsigned long blLastConnectAttempt = millis() - 60000;
            if (timePassedSince(blLastConnectAttempt)<60000){
              log += "skip connect to blynk server too often. Wait a little...";
              addLog(LOG_LEVEL_INFO, log);
              return true;
            }
            blLastConnectAttempt=millis();

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

        if (!Blynk.connected()){
          return true;
        }

        // Collect the values at the same run, to make sure all are from the same sample
        byte valueCount = getValueCountFromSensorType(event->sensorType);
        C014_queue_element element(event, valueCount);
        if (ExtraTaskSettings.TaskIndex != event->TaskIndex)
          PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummyString);

        MakeControllerSettings(ControllerSettings);
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);

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
          String log = F("BL: ");
          if (vPinNumber>0 && vPinNumber<256){
            log += "sending ";
            log += valueFullName;
            log += " value ";
            log += formattedValue;
            log += " to blynk pin v";
            log += vPinNumber;
            // Blynk.virtualWrite(vPinNumber, formattedValue);
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
        // return true;
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

  // if (!try_connect_host(controller_number, client, ControllerSettings))
  //   return false;

bool do_process_c014_delay_queue(int controller_number, const C014_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  while (element.txt[element.valuesSent] == "" || element.vPin[element.valuesSent] == -1) {
  //   // A non valid value, which we are not going to send.
  //   // Increase sent counter until a valid value is found.
    if (element.checkDone(true))
      return true;
  }
  if (wifiStatus != ESPEASY_WIFI_SERVICES_INITIALIZED) {
    return false;
  }
  return element.checkDone(Blynk_get_c014(element.txt[element.valuesSent], element.vPin[element.valuesSent]));
}


boolean Blynk_get_c014(const String& value, int vPin )
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
// Blynk.syncAll();
addLog(LOG_LEVEL_INFO, F("BL: connected handler"));
}


// This is called when Smartphone App is opened
BLYNK_APP_CONNECTED() {
  addLog(LOG_LEVEL_INFO, F("BL: app connected handler"));
}

// This is called when Smartphone App is closed
BLYNK_APP_DISCONNECTED() {
  addLog(LOG_LEVEL_INFO, F("BL: app disconnected handler"));
}



#endif
