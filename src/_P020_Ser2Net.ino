#include "_Plugin_Helper.h"
#ifdef USES_P020

// #######################################################################################################
// #################################### Plugin 020: Ser2Net ##############################################
// #######################################################################################################


#define PLUGIN_020
#define PLUGIN_ID_020         20
#define PLUGIN_NAME_020       "Communication - Serial Server"
#define PLUGIN_VALUENAME1_020 "Ser2Net"

#define P020_BUFFER_SIZE 128
boolean Plugin_020_init             = false;
byte    Plugin_020_SerialProcessing = 0;

WiFiServer *ser2netServer;
WiFiClient  ser2netClient;

boolean Plugin_020(byte function, struct EventStruct *event, String& string)
{
  boolean success             = false;
  static byte connectionState = 0;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number    = PLUGIN_ID_020;
      Device[deviceCount].Type        = DEVICE_TYPE_SINGLE;
      Device[deviceCount].Custom      = true;
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
      addFormNumericBox(F("TCP Port"),  F("p020_port"), ExtraTaskSettings.TaskDevicePluginConfigLong[0]);
      addFormNumericBox(F("Baud Rate"), F("p020_baud"), ExtraTaskSettings.TaskDevicePluginConfigLong[1]);
      addFormNumericBox(F("Data bits"), F("p020_data"), ExtraTaskSettings.TaskDevicePluginConfigLong[2]);

      byte   choice = ExtraTaskSettings.TaskDevicePluginConfigLong[3];
      String options[3];
      options[0] = F("No parity");
      options[1] = F("Even");
      options[2] = F("Odd");
      int optionValues[3];
      optionValues[0] = 0;
      optionValues[1] = 2;
      optionValues[2] = 3;
      addFormSelector(F("Parity"), F("p020_parity"), 3, options, optionValues, choice);

      addFormNumericBox(F("Stop bits"), F("p020_stop"), ExtraTaskSettings.TaskDevicePluginConfigLong[4]);

      addFormPinSelect(F("TX Enable Pin"),           F("taskdevicepin2"), Settings.TaskDevicePin2[event->TaskIndex]);

      addFormPinSelect(F("Reset target after boot"), F("taskdevicepin1"), Settings.TaskDevicePin1[event->TaskIndex]);

      addFormNumericBox(F("RX Receive Timeout (mSec)"), F("p020_rxwait"), PCONFIG(0));


      byte   choice2 = PCONFIG(1);
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
      ExtraTaskSettings.TaskDevicePluginConfigLong[0]      = getFormItemInt(F("p020_port"));
      ExtraTaskSettings.TaskDevicePluginConfigLong[1]      = getFormItemInt(F("p020_baud"));
      ExtraTaskSettings.TaskDevicePluginConfigLong[2]      = getFormItemInt(F("p020_data"));
      ExtraTaskSettings.TaskDevicePluginConfigLong[3]      = getFormItemInt(F("p020_parity"));
      ExtraTaskSettings.TaskDevicePluginConfigLong[4]      = getFormItemInt(F("p020_stop"));
      PCONFIG(0) = getFormItemInt(F("p020_rxwait"));
      PCONFIG(1) = getFormItemInt(F("p020_events"));
      success                                              = true;
      break;
    }

    case PLUGIN_INIT:
    {
      LoadTaskSettings(event->TaskIndex);

      if ((ExtraTaskSettings.TaskDevicePluginConfigLong[0] != 0) && (ExtraTaskSettings.TaskDevicePluginConfigLong[1] != 0))
      {
          #if defined(ESP8266)
        byte serialconfig = 0x10;
          #endif // if defined(ESP8266)
          #if defined(ESP32)
        uint32_t serialconfig = 0x8000010;
          #endif // if defined(ESP32)
        serialconfig += ExtraTaskSettings.TaskDevicePluginConfigLong[3];
        serialconfig += (ExtraTaskSettings.TaskDevicePluginConfigLong[2] - 5) << 2;

        if (ExtraTaskSettings.TaskDevicePluginConfigLong[4] == 2) {
          serialconfig += 0x20;
        }
          #if defined(ESP8266)
        Serial.begin(ExtraTaskSettings.TaskDevicePluginConfigLong[1], (SerialConfig)serialconfig);
          #endif // if defined(ESP8266)
          #if defined(ESP32)
        Serial.begin(ExtraTaskSettings.TaskDevicePluginConfigLong[1], serialconfig);
          #endif // if defined(ESP32)
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

        // set the TX Enable Pin to LOW
        if (Settings.TaskDevicePin2[event->TaskIndex] != -1)
        {
          pinMode(Settings.TaskDevicePin2[event->TaskIndex], OUTPUT);
          digitalWrite(Settings.TaskDevicePin2[event->TaskIndex], LOW);
        }

        Plugin_020_init = true;
      }
      Plugin_020_SerialProcessing = PCONFIG(1);
      success                     = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      if (Plugin_020_init)
      {
        size_t bytes_read;

        if (ser2netServer->hasClient())
        {
          if (ser2netClient) { ser2netClient.stop(); }
          ser2netClient = ser2netServer->available();
          addLog(LOG_LEVEL_ERROR, F("Ser2N: Client connected!"));
        }

        if (ser2netClient.connected())
        {
          connectionState = 1;
          uint8_t net_buf[P020_BUFFER_SIZE];
          int     count = ser2netClient.available();

          if (count > 0)
          {
            if (count > P020_BUFFER_SIZE) {
              count = P020_BUFFER_SIZE;
            }
            bytes_read = ser2netClient.read(net_buf, count);

            if (Settings.TaskDevicePin2[event->TaskIndex] != -1)
            {
              digitalWrite(Settings.TaskDevicePin2[event->TaskIndex], HIGH); // Activate the TX Enable
            }
            Serial.write(net_buf, bytes_read);
            Serial.flush();                                                  // Waits for the transmission of outgoing serial data to
                                                                             // complete

            if (Settings.TaskDevicePin2[event->TaskIndex] != -1)
            {
              digitalWrite(Settings.TaskDevicePin2[event->TaskIndex], LOW); // Deactivate the TX Enable
            }

            if (count == P020_BUFFER_SIZE)                                  // if we have a full buffer, drop the last position to stuff
                                                                            // with string end marker
            {
              count--;
              addLog(LOG_LEVEL_ERROR, F("Ser2N: network buffer full!"));
            }
            net_buf[count] = 0; // before logging as a char array, zero terminate the last position to be safe.
            char log[P020_BUFFER_SIZE + 40];
            sprintf_P(log, PSTR("Ser2N: N>: %s"), (char *)net_buf);
            addLog(LOG_LEVEL_DEBUG, log);
          }
        }
        else
        {
          if (connectionState == 1) // there was a client connected before...
          {
            connectionState = 0;

            // workaround see: https://github.com/esp8266/Arduino/issues/4497#issuecomment-373023864
            //ser2netClient = WiFiClient();
            ser2netClient.setTimeout(CONTROLLER_CLIENTTIMEOUT_DFLT);
            addLog(LOG_LEVEL_ERROR, F("Ser2N: Client disconnected!"));
          }

          while (Serial.available()) {
            Serial.read();
          }
        }

        success = true;
      }
      break;
    }

    case PLUGIN_SERIAL_IN:
    {
      if (Plugin_020_init)
      {
        uint8_t serial_buf[P020_BUFFER_SIZE];
        int     RXWait = PCONFIG(0);

        if (RXWait == 0) {
          RXWait = 1;
        }
        int timeOut       = RXWait;
        size_t bytes_read = 0;

        PrepareSend();
        while (timeOut > 0)
        {
          while (Serial.available()) {
            if (bytes_read < P020_BUFFER_SIZE) {
              serial_buf[bytes_read] = Serial.read();
              bytes_read++;
            }
            else {
              Serial.read();  // when the buffer is full, just read remaining input, but do not store...
            }
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
              ser2netClient.write((const uint8_t *)serial_buf, bytes_read);
              ser2netClient.flush();
            }
          }
        }
        else // if we have a full buffer, drop the last position to stuff with string end marker
        {
          while (Serial.available()) { // read possible remaining data to avoid sending rubbish...
            Serial.read();
          }
          bytes_read--;

          // and log buffer full situation
          addLog(LOG_LEVEL_ERROR, F("Ser2N: serial buffer full!"));
        }
        serial_buf[bytes_read] = 0; // before logging as a char array, zero terminate the last position to be safe.
        char log[P020_BUFFER_SIZE + 40];
        sprintf_P(log, PSTR("Ser2N: S>: %s"), (char *)serial_buf);
        addLog(LOG_LEVEL_DEBUG, log);

        // We can also use the rules engine for local control!
        if (Settings.UseRules)
        {
          String message    = (char *)serial_buf;
          int    NewLinePos = message.indexOf(F("\r\n"));

          if (NewLinePos > 0) {
            message = message.substring(0, NewLinePos);
          }
          String eventString;

          switch (Plugin_020_SerialProcessing)
          {
            case 0:
            {
              break;
            }

            case 1: // Generic
            {
              eventString  = F("!Serial#");
              eventString += message;
              break;
            }

            case 2:                                 // RFLink
            {
              message = message.substring(6);       // RFLink, strip 20;xx; from incoming message

              if (message.startsWith("ESPEASY"))    // Special treatment for gpio values, strip unneeded parts...
              {
                message     = message.substring(8); // Strip "ESPEASY;"
                eventString = F("RFLink#");
              }
              else {
                eventString = F("!RFLink#"); // default event as it comes in, literal match needed in rules, using '!'
              }
              eventString += message;
              break;
            }
          } // switch

          if (eventString.length() > 0) {
            eventQueue.add(eventString);
          }
        } // if rules
        success = true;
      }
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
