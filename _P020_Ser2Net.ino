//#######################################################################################################
//#################################### Plugin 020: Ser2Net ##############################################
//#######################################################################################################

#define PLUGIN_020
#define PLUGIN_ID_020         20
#define PLUGIN_NAME_020       "Serial Server"
#define PLUGIN_VALUENAME1_020 "Ser2Net"

#define BUFFER_SIZE 128
boolean Plugin_020_init = false;

WiFiServer *ser2netServer;
WiFiClient ser2netClient;

boolean Plugin_020(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte connectionState=0;

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

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_020_port");
        ExtraTaskSettings.TaskDevicePluginConfigLong[0] = plugin1.toInt();
        String plugin2 = WebServer.arg("plugin_020_baud");
        ExtraTaskSettings.TaskDevicePluginConfigLong[1] = plugin2.toInt();
        String plugin3 = WebServer.arg("plugin_020_data");
        ExtraTaskSettings.TaskDevicePluginConfigLong[2] = plugin3.toInt();
        String plugin4 = WebServer.arg("plugin_020_parity");
        ExtraTaskSettings.TaskDevicePluginConfigLong[3] = plugin4.toInt();
        String plugin5 = WebServer.arg("plugin_020_stop");
        ExtraTaskSettings.TaskDevicePluginConfigLong[4] = plugin5.toInt();
        String plugin6 = WebServer.arg("plugin_020_rxwait");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin6.toInt();
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
            char log[40];
            strcpy_P(log, PSTR("Ser2N: Client connected!"));
            addLog(LOG_LEVEL_ERROR, log);
          }

          if (ser2netClient.connected())
          {
            connectionState=1;
            uint8_t net_buf[BUFFER_SIZE];
            int count = ser2netClient.available();
            if (count > 0)
            {
              if (count > BUFFER_SIZE)
                count = BUFFER_SIZE;
              bytes_read = ser2netClient.read(net_buf, count);
              Serial.write(net_buf, bytes_read);
              Serial.flush(); // Waits for the transmission of outgoing serial data to complete

              if (count == BUFFER_SIZE) // if we have a full buffer, drop the last position to stuff with string end marker
              {
                count--;
                char log[40];
                strcpy_P(log, PSTR("Ser2N: network buffer full!"));   // and log buffer full situation
                addLog(LOG_LEVEL_ERROR, log);
              }
              net_buf[count]=0; // before logging as a char array, zero terminate the last position to be safe.
              char log[BUFFER_SIZE+40];
              sprintf_P(log, PSTR("Ser2N: N>: %s"), (char*)net_buf);
              addLog(LOG_LEVEL_DEBUG,log);
            }
          }
          else
          {
            if(connectionState == 1) // there was a client connected before...
            {
              connectionState=0;
              char log[40];
              strcpy_P(log, PSTR("Ser2N: Client disconnected!"));
              addLog(LOG_LEVEL_ERROR, log);
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
        if (Plugin_020_init)
        {
          if (ser2netClient.connected())
          {
            uint8_t serial_buf[BUFFER_SIZE];
            int RXWait = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
            if (RXWait == 0)
              RXWait = 1;
            int timeOut = RXWait;
            size_t bytes_read = 0;
            while (timeOut > 0)
            {
              while (Serial.available()) {
                if (bytes_read < BUFFER_SIZE) {
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
            
            if (bytes_read != BUFFER_SIZE)
            {
              if (bytes_read > 0) {
                ser2netClient.write((const uint8_t*)serial_buf, bytes_read);
                ser2netClient.flush();
              }
            }
            else // if we have a full buffer, drop the last position to stuff with string end marker
            {
              while (Serial.available()) // read possible remaining data to avoid sending rubbish...
                Serial.read();
              bytes_read--;
              char log[40];
              strcpy_P(log, PSTR("Ser2N: serial buffer full!"));   // and log buffer full situation
              addLog(LOG_LEVEL_ERROR, log);
            }  
            serial_buf[bytes_read]=0; // before logging as a char array, zero terminate the last position to be safe.
            char log[BUFFER_SIZE+40];
            sprintf_P(log, PSTR("Ser2N: S>: %s"), (char*)serial_buf);
            addLog(LOG_LEVEL_DEBUG,log);
          }
          success = true;
        }
        break;
      }

  }
  return success;
}

