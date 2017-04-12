//#######################################################################################################
//#################################### Plugin 020: Ser2Net ##############################################
//#######################################################################################################

#define PLUGIN_020
#define PLUGIN_ID_020         20
#define PLUGIN_NAME_020       "Serial Server"
#define PLUGIN_VALUENAME1_020 "Ser2Net"

#define P020_BUFFER_SIZE 128
boolean Plugin_020_init = false;
byte Plugin_020_SerialProcessing = 0;

WiFiServer *ser2netServer;
WiFiClient ser2netClient;

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

    case PLUGIN_WEBFORM_LOAD:
      {
        char tmpString[128];
        sprintf_P(tmpString, PSTR("<TR><TD>TCP Port:<TD><input type='text' name='plugin_020_port' value='%u'>"), ExtraTaskSettings.TaskDevicePluginConfigLong[0]);
        string += tmpString;
        sprintf_P(tmpString, PSTR("<TR><TD>Baud Rate:<TD><input type='text' name='plugin_020_baud' value='%u'>"), ExtraTaskSettings.TaskDevicePluginConfigLong[1]);
        string += tmpString;
        sprintf_P(tmpString, PSTR("<TR><TD>Data bits:<TD><input type='text' name='plugin_020_data' value='%u'>"), ExtraTaskSettings.TaskDevicePluginConfigLong[2]);
        string += tmpString;

        byte choice = ExtraTaskSettings.TaskDevicePluginConfigLong[3];
        String options[3];
        options[0] = F("No parity");
        options[1] = F("Even");
        options[2] = F("Odd");
        int optionValues[3];
        optionValues[0] = 0;
        optionValues[1] = 2;
        optionValues[2] = 3;
        string += F("<TR><TD>Parity:<TD><select name='plugin_020_parity'>");
        for (byte x = 0; x < 3; x++)
        {
          string += F("<option value='");
          string += optionValues[x];
          string += "'";
          if (choice == optionValues[x])
            string += F(" selected");
          string += ">";
          string += options[x];
          string += F("</option>");
        }
        string += F("</select>");

        sprintf_P(tmpString, PSTR("<TR><TD>Stop bits:<TD><input type='text' name='plugin_020_stop' value='%u'>"), ExtraTaskSettings.TaskDevicePluginConfigLong[4]);
        string += tmpString;

        string += F("<TR><TD>Reset target after boot:<TD>");
        addPinSelect(false, string, "taskdevicepin1", Settings.TaskDevicePin1[event->TaskIndex]);

        sprintf_P(tmpString, PSTR("<TR><TD>RX Receive Timeout (mSec):<TD><input type='text' name='plugin_020_rxwait' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
        string += tmpString;

        byte choice2 = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        String options2[3];
        options2[0] = F("None");
        options2[1] = F("Generic");
        options2[2] = F("RFLink");
        int optionValues2[3];
        optionValues2[0] = 0;
        optionValues2[1] = 1;
        optionValues2[2] = 2;
        string += F("<TR><TD>Event processing:<TD><select name='plugin_020_events'>");
        for (byte x = 0; x < 3; x++)
      {
        string += F("<option value='");
          string += optionValues2[x];
          string += "'";
          if (choice2 == optionValues2[x])
            string += F(" selected");
          string += ">";
          string += options2[x];
          string += F("</option>");
        }
        string += F("</select>");

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg(F("plugin_020_port"));
        ExtraTaskSettings.TaskDevicePluginConfigLong[0] = plugin1.toInt();
        String plugin2 = WebServer.arg(F("plugin_020_baud"));
        ExtraTaskSettings.TaskDevicePluginConfigLong[1] = plugin2.toInt();
        String plugin3 = WebServer.arg(F("plugin_020_data"));
        ExtraTaskSettings.TaskDevicePluginConfigLong[2] = plugin3.toInt();
        String plugin4 = WebServer.arg(F("plugin_020_parity"));
        ExtraTaskSettings.TaskDevicePluginConfigLong[3] = plugin4.toInt();
        String plugin5 = WebServer.arg(F("plugin_020_stop"));
        ExtraTaskSettings.TaskDevicePluginConfigLong[4] = plugin5.toInt();
        String plugin6 = WebServer.arg(F("plugin_020_rxwait"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin6.toInt();
        String plugin7 = WebServer.arg(F("plugin_020_events"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin7.toInt();
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        LoadTaskSettings(event->TaskIndex);
        if ((ExtraTaskSettings.TaskDevicePluginConfigLong[0] != 0) && (ExtraTaskSettings.TaskDevicePluginConfigLong[1] != 0))
        {
          byte serialconfig = 0x10;
          serialconfig += ExtraTaskSettings.TaskDevicePluginConfigLong[3];
          serialconfig += (ExtraTaskSettings.TaskDevicePluginConfigLong[2] - 5) << 2;
          if (ExtraTaskSettings.TaskDevicePluginConfigLong[4] == 2)
            serialconfig += 0x20;
          Serial.begin(ExtraTaskSettings.TaskDevicePluginConfigLong[1], (SerialConfig)serialconfig);
          ser2netServer = new WiFiServer(ExtraTaskSettings.TaskDevicePluginConfigLong[0]);
          ser2netServer->begin();

          if (Settings.TaskDevicePin1[event->TaskIndex] != -1)
          {
            pinMode(Settings.TaskDevicePin1[event->TaskIndex], OUTPUT);
            digitalWrite(Settings.TaskDevicePin1[event->TaskIndex], LOW);
            delay(500);
            digitalWrite(Settings.TaskDevicePin1[event->TaskIndex], HIGH);
            pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
          }

          Plugin_020_init = true;
        }
        Plugin_020_SerialProcessing = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_020_init)
        {
          size_t bytes_read;
          if (ser2netServer->hasClient())
          {
            if (ser2netClient) ser2netClient.stop();
            ser2netClient = ser2netServer->available();
            addLog(LOG_LEVEL_ERROR, F("Ser2N: Client connected!"));
          }

          if (ser2netClient.connected())
          {
            connectionState = 1;
            uint8_t net_buf[P020_BUFFER_SIZE];
            int count = ser2netClient.available();
            if (count > 0)
            {
              if (count > P020_BUFFER_SIZE)
                count = P020_BUFFER_SIZE;
              bytes_read = ser2netClient.read(net_buf, count);
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
              addLog(LOG_LEVEL_ERROR, F("Ser2N: Client disconnected!"));
            }

            while (Serial.available())
              Serial.read();
          }

          success = true;
        }
        break;
      }

    case PLUGIN_SERIAL_IN:
      {
        uint8_t serial_buf[P020_BUFFER_SIZE];
        int RXWait = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
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
            if (Plugin_020_init && ser2netClient.connected())
            {
              ser2netClient.write((const uint8_t*)serial_buf, bytes_read);
              ser2netClient.flush();
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

          switch (Plugin_020_SerialProcessing)
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
          Serial.println(tmpString);
        }
        break;
      }

  }
  return success;
}
