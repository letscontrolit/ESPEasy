#ifdef USES_P020
//#######################################################################################################
//#################################### Plugin 020: Ser2Net ##############################################
//#######################################################################################################

#define PLUGIN_020
#define PLUGIN_ID_020         20
#define PLUGIN_NAME_020       "Communication - Serial Server"
#define PLUGIN_VALUENAME1_020 "Ser2Net"

#define P020_BUFFER_SIZE 128

struct P020_data_struct : public PluginTaskData_base {

  P020_data_struct(unsigned long port) {
    ser2netServer = new WiFiServer(port);
    ser2netServer->begin();
  }

  ~P020_data_struct() {
    if (nullptr != ser2netServer) {
      delete ser2netServer;
      ser2netServer = nullptr;
    }
  }

  WiFiServer *ser2netServer = nullptr;
  WiFiClient ser2netClient;
  byte SerialProcessing = 0;
};

boolean Plugin_020(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte connectionState = 0;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_020;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].Custom = true;
        Device[deviceCount].TimerOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_020);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_020));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_bidirectional(F("Reset"));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        LoadTaskSettings(event->TaskIndex);
      	addFormNumericBox(F("TCP Port"), F("p020_port"), ExtraTaskSettings.TaskDevicePluginConfigLong[0]);
      	addFormNumericBox(F("Baud Rate"), F("p020_baud"), ExtraTaskSettings.TaskDevicePluginConfigLong[1]);

        byte serialConfChoice = serialHelper_convertOldSerialConfig(PCONFIG(2));
        serialHelper_serialconfig_webformLoad(event, serialConfChoice);

      	addFormPinSelect(F("Reset target after boot"), F("taskdevicepin1"), CONFIG_PIN1);

      	addFormNumericBox(F("RX Receive Timeout (mSec)"), F("p020_rxwait"), PCONFIG(0));

        byte choice2 = PCONFIG(1);
        String options2[3];
        options2[0] = F("None");
        options2[1] = F("Generic");
        options2[2] = F("RFLink");
        addFormSelector(F("Event processing"), F("p020_events"), 3, options2, NULL, choice2);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        LoadTaskSettings(event->TaskIndex);
        ExtraTaskSettings.TaskDevicePluginConfigLong[0] = getFormItemInt(F("p020_port"));
        ExtraTaskSettings.TaskDevicePluginConfigLong[1] = getFormItemInt(F("p020_baud"));
        PCONFIG(0) = getFormItemInt(F("p020_rxwait"));
        PCONFIG(1) = getFormItemInt(F("p020_events"));
        PCONFIG(2) = serialHelper_serialconfig_webformSave();
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        initPluginTaskData(event->TaskIndex, new P020_data_struct(ExtraTaskSettings.TaskDevicePluginConfigLong[0]));
        P020_data_struct *P020_data =
            static_cast<P020_data_struct *>(getPluginTaskData(event->TaskIndex));
        if (nullptr == P020_data) {
          break;
        }
        LoadTaskSettings(event->TaskIndex);
        if ((ExtraTaskSettings.TaskDevicePluginConfigLong[0] != 0) && (ExtraTaskSettings.TaskDevicePluginConfigLong[1] != 0))
        {

      #if defined(ESP8266)
           byte serialconfig = 0;
      #elif defined(ESP32)
           uint32_t serialconfig = 0x8000000;
      #endif
          serialconfig |= serialHelper_convertOldSerialConfig(PCONFIG(2));
      #if defined(ESP8266)
          Serial.begin(ExtraTaskSettings.TaskDevicePluginConfigLong[1], (SerialConfig)serialconfig);
      #elif defined(ESP32)
          Serial.begin(ExtraTaskSettings.TaskDevicePluginConfigLong[1], serialconfig);
      #endif

          if (CONFIG_PIN1 != -1)
          {
            pinMode(CONFIG_PIN1, OUTPUT);
            digitalWrite(CONFIG_PIN1, LOW);
            delay(500);
            digitalWrite(CONFIG_PIN1, HIGH);
            pinMode(CONFIG_PIN1, INPUT_PULLUP);
          }
        }
        P020_data->SerialProcessing = PCONFIG(1);
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        P020_data_struct *P020_data =
            static_cast<P020_data_struct *>(getPluginTaskData(event->TaskIndex));
        if (nullptr == P020_data) {
          break;
        }
        size_t bytes_read;
        if (P020_data->ser2netServer->hasClient())
        {
          if (P020_data->ser2netClient) P020_data->ser2netClient.stop();
          P020_data->ser2netClient = P020_data->ser2netServer->available();
          addLog(LOG_LEVEL_ERROR, F("Ser2N: Client connected!"));
        }

        if (P020_data->ser2netClient.connected())
        {
          connectionState = 1;
          uint8_t net_buf[P020_BUFFER_SIZE];
          int count = P020_data->ser2netClient.available();
          if (count > 0)
          {
            if (count > P020_BUFFER_SIZE)
              count = P020_BUFFER_SIZE;
            bytes_read = P020_data->ser2netClient.read(net_buf, count);
            Serial.write(net_buf, bytes_read);
            Serial.flush(); // Waits for the transmission of outgoing serial data to complete

            if (count == P020_BUFFER_SIZE) // if we have a full buffer, drop the last position to stuff with string end marker
            {
              count--;
              addLog(LOG_LEVEL_ERROR, F("Ser2N: network buffer full!"));
            }
            net_buf[count] = 0; // before logging as a char array, zero terminate the last position to be safe.
            char log[P020_BUFFER_SIZE + 40];
            sprintf_P(log, PSTR("Ser2N: N>: %s"), (char*)net_buf);
            addLog(LOG_LEVEL_DEBUG, log);
          }
        }
        else
        {
          if (connectionState == 1) // there was a client connected before...
          {
            connectionState = 0;
            // workaround see: https://github.com/esp8266/Arduino/issues/4497#issuecomment-373023864
            P020_data->ser2netClient = WiFiClient();
            addLog(LOG_LEVEL_ERROR, F("Ser2N: Client disconnected!"));
          }

          while (Serial.available())
            Serial.read();
        }

        success = true;
        break;
      }

    case PLUGIN_SERIAL_IN:
      {
        P020_data_struct *P020_data =
            static_cast<P020_data_struct *>(getPluginTaskData(event->TaskIndex));
        if (nullptr == P020_data) {
          break;
        }
        uint8_t serial_buf[P020_BUFFER_SIZE];
        int RXWait = PCONFIG(0);
        if (RXWait == 0)
          RXWait = 1;
        int timeOut = RXWait;
        size_t bytes_read = 0;
        while (timeOut > 0)
        {
          while (Serial.available()) {
            if (bytes_read < P020_BUFFER_SIZE) {
              serial_buf[bytes_read] = Serial.read();
              bytes_read++;
            }
            else
              Serial.read();  // when the buffer is full, just read remaining input, but do not store...

            timeOut = RXWait; // if serial received, reset timeout counter
          }
          delay(1);
          timeOut--;
        }

        if (bytes_read != P020_BUFFER_SIZE)
        {
          if (bytes_read > 0) {
            if (P020_data->ser2netClient.connected())
            {
              P020_data->ser2netClient.write((const uint8_t*)serial_buf, bytes_read);
              P020_data->ser2netClient.flush();
            }
          }
        }
        else // if we have a full buffer, drop the last position to stuff with string end marker
        {
          while (Serial.available()) // read possible remaining data to avoid sending rubbish...
            Serial.read();
          bytes_read--;
          // and log buffer full situation
          addLog(LOG_LEVEL_ERROR, F("Ser2N: serial buffer full!"));
        }
        serial_buf[bytes_read] = 0; // before logging as a char array, zero terminate the last position to be safe.
        char log[P020_BUFFER_SIZE + 40];
        sprintf_P(log, PSTR("Ser2N: S>: %s"), (char*)serial_buf);
        addLog(LOG_LEVEL_DEBUG, log);

        // We can also use the rules engine for local control!
        if (Settings.UseRules)
        {
          String message = (char*)serial_buf;
          int NewLinePos = message.indexOf("\r\n");
          if (NewLinePos > 0)
            message = message.substring(0, NewLinePos);
          String eventString = "";

          switch (P020_data->SerialProcessing)
          {
            case 0:
              {
                break;
              }

            case 1: // Generic
              {
                eventString = F("!Serial#");
                eventString += message;
                break;
              }

            case 2: // RFLink
              {
                message = message.substring(6); // RFLink, strip 20;xx; from incoming message
                if (message.startsWith("ESPEASY")) // Special treatment for gpio values, strip unneeded parts...
                {
                  message = message.substring(8); // Strip "ESPEASY;"
                  eventString = F("RFLink#");
                }
                else
                  eventString = F("!RFLink#"); // default event as it comes in, literal match needed in rules, using '!'
                eventString += message;
                break;
              }
          } // switch

          if (eventString.length() > 0)
            rulesProcessing(eventString);

        } // if rules
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);
        if (command == F("serialsend"))
        {
          success = true;
          String tmpString = string.substring(11);
          Serial.println(tmpString); // FIXME TD-er: Should this also use the serial write buffer?
        }
        break;
      }

  }
  return success;
}
#endif // USES_P020
