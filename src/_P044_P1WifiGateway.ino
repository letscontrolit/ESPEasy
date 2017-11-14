//#################################### Plugin 044: P1WifiGateway ########################################
//
//  based on P020 Ser2Net, extended by Ronald Leenes romix/-at-/macuser.nl
//
//  designed for combo
//    Wemos D1 mini (see http://wemos.cc) and
//    P1 wifi gateway shield (see https://circuits.io/circuits/2460082)
//    see http://romix.macuser.nl for kits
//#######################################################################################################

#define PLUGIN_044
#define PLUGIN_ID_044         44
#define PLUGIN_NAME_044       "Communication - P1 Wifi Gateway"
#define PLUGIN_VALUENAME1_044 "P1WifiGateway"

#define STATUS_LED 12
#define P044_BUFFER_SIZE 1024
#define NETBUF_SIZE 600
#define DISABLED 0
#define WAITING 1
#define READING 2
#define CHECKSUM 3
#define DONE 4

boolean Plugin_044_init = false;
boolean serialdebug = false;
char* Plugin_044_serial_buf;
unsigned int bytes_read = 0;
boolean CRCcheck = false;
unsigned int currCRC = 0;
int checkI = 0;

WiFiServer *P1GatewayServer;
WiFiClient P1GatewayClient;

boolean Plugin_044(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte connectionState = 0;
  static int state = DISABLED;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_044;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].Custom = true;
        Device[deviceCount].TimerOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_044);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_044));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
      	addFormNumericBox(string, F("TCP Port"), F("plugin_044_port"), ExtraTaskSettings.TaskDevicePluginConfigLong[0]);
      	addFormNumericBox(string, F("Baud Rate"), F("plugin_044_baud"), ExtraTaskSettings.TaskDevicePluginConfigLong[1]);
      	addFormNumericBox(string, F("Data bits"), F("plugin_044_data"), ExtraTaskSettings.TaskDevicePluginConfigLong[2]);

        byte choice = ExtraTaskSettings.TaskDevicePluginConfigLong[3];
        String options[3];
        options[0] = F("No parity");
        options[1] = F("Even");
        options[2] = F("Odd");
        int optionValues[3] = { 0, 2, 3 };
        addFormSelector(string, F("Parity"), F("plugin_044_parity"), 3, options, optionValues, choice);

      	addFormNumericBox(string, F("Stop bits"), F("plugin_044_stop"), ExtraTaskSettings.TaskDevicePluginConfigLong[4]);

      	addFormPinSelect(string, F("Reset target after boot"), F("taskdevicepin1"), Settings.TaskDevicePin1[event->TaskIndex]);

      	addFormNumericBox(string, F("RX Receive Timeout (mSec)"), F("plugin_044_rxwait"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        ExtraTaskSettings.TaskDevicePluginConfigLong[0] = getFormItemInt(F("plugin_044_port"));
        ExtraTaskSettings.TaskDevicePluginConfigLong[1] = getFormItemInt(F("plugin_044_baud"));
        ExtraTaskSettings.TaskDevicePluginConfigLong[2] = getFormItemInt(F("plugin_044_data"));
        ExtraTaskSettings.TaskDevicePluginConfigLong[3] = getFormItemInt(F("plugin_044_parity"));
        ExtraTaskSettings.TaskDevicePluginConfigLong[4] = getFormItemInt(F("plugin_044_stop"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_044_rxwait"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        pinMode(STATUS_LED, OUTPUT);
        digitalWrite(STATUS_LED, 0);

        LoadTaskSettings(event->TaskIndex);
        if ((ExtraTaskSettings.TaskDevicePluginConfigLong[0] != 0) && (ExtraTaskSettings.TaskDevicePluginConfigLong[1] != 0))
        {
          byte serialconfig = 0x10;
          serialconfig += ExtraTaskSettings.TaskDevicePluginConfigLong[3];
          serialconfig += (ExtraTaskSettings.TaskDevicePluginConfigLong[2] - 5) << 2;
          if (ExtraTaskSettings.TaskDevicePluginConfigLong[4] == 2)
            serialconfig += 0x20;
          Serial.begin(ExtraTaskSettings.TaskDevicePluginConfigLong[1], (SerialConfig)serialconfig);
          P1GatewayServer = new WiFiServer(ExtraTaskSettings.TaskDevicePluginConfigLong[0]);
          P1GatewayServer->begin();

          if (!Plugin_044_serial_buf)
            Plugin_044_serial_buf = (char *)malloc(P044_BUFFER_SIZE);

          if (Settings.TaskDevicePin1[event->TaskIndex] != -1)
          {
            pinMode(Settings.TaskDevicePin1[event->TaskIndex], OUTPUT);
            digitalWrite(Settings.TaskDevicePin1[event->TaskIndex], LOW);
            delay(500);
            digitalWrite(Settings.TaskDevicePin1[event->TaskIndex], HIGH);
            pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
          }

          Plugin_044_init = true;
        }

        blinkLED();

        if (ExtraTaskSettings.TaskDevicePluginConfigLong[1] == 115200) {
          addLog(LOG_LEVEL_DEBUG, F("P1   : DSMR version 4 meter, CRC on"));
          CRCcheck = true;
        } else {
          addLog(LOG_LEVEL_DEBUG, F("P1   : DSMR version 4 meter, CRC on"));
          CRCcheck = false;
        }


        state = WAITING;
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_044_init)
        {
          size_t bytes_read;
          if (P1GatewayServer->hasClient())
          {
            if (P1GatewayClient) P1GatewayClient.stop();
            P1GatewayClient = P1GatewayServer->available();
            addLog(LOG_LEVEL_ERROR, F("P1   : Client connected!"));
          }

          if (P1GatewayClient.connected())
          {
            connectionState = 1;
            uint8_t net_buf[P044_BUFFER_SIZE];
            int count = P1GatewayClient.available();
            if (count > 0)
            {
              if (count > P044_BUFFER_SIZE)
                count = P044_BUFFER_SIZE;
              bytes_read = P1GatewayClient.read(net_buf, count);
              Serial.write(net_buf, bytes_read);
              Serial.flush(); // Waits for the transmission of outgoing serial data to complete

              if (count == P044_BUFFER_SIZE) // if we have a full buffer, drop the last position to stuff with string end marker
              {
                count--;
                // and log buffer full situation
                addLog(LOG_LEVEL_ERROR, F("P1   : Error: network buffer full!"));
              }
              net_buf[count] = 0; // before logging as a char array, zero terminate the last position to be safe.
              char log[P044_BUFFER_SIZE + 40];
              sprintf_P(log, PSTR("P1   : Error: N>: %s"), (char*)net_buf);
              addLog(LOG_LEVEL_DEBUG, log);
            }
          }
          else
          {
            if (connectionState == 1) // there was a client connected before...
            {
              connectionState = 0;
              addLog(LOG_LEVEL_ERROR, F("P1   : Client disconnected!"));
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
        if (Plugin_044_init)
        {
          if (P1GatewayClient.connected())
          {
            int RXWait = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
            if (RXWait == 0)
              RXWait = 1;
            int timeOut = RXWait;
            while (timeOut > 0)
            {
              while (Serial.available() && state != DONE) {
                if (bytes_read < P044_BUFFER_SIZE - 5) {
                  char  ch = Serial.read();
                  digitalWrite(STATUS_LED, 1);
                  switch (state) {
                    case DISABLED: //ignore incoming data
                      break;
                    case WAITING:
                      if (ch == '/')  {
                        Plugin_044_serial_buf[bytes_read] = ch;
                        bytes_read++;
                        state = READING;
                      } // else ignore data
                      break;
                    case READING:
                      if (ch == '!') {
                        if (CRCcheck) {
                          state = CHECKSUM;
                        } else {
                          state = DONE;
                        }
                      }
                      if (validP1char(ch)) {
                        Plugin_044_serial_buf[bytes_read] = ch;
                        bytes_read++;
                      } else {              // input is non-ascii
                        addLog(LOG_LEVEL_DEBUG, F("P1   : Error: DATA corrupt, discarded input."));
                        Serial.flush();
                        bytes_read = 0;
                        state = WAITING;
                      }
                      break;
                    case CHECKSUM:
                      checkI ++;
                      if (checkI == 5) {
                        checkI = 0;
                        state = DONE;
                      } else {
                        Plugin_044_serial_buf[bytes_read] = ch;
                        bytes_read++;
                      }
                      break;
                    case DONE:
                      // Plugin_044_serial_buf[bytes_read]= '\n';
                      // bytes_read++;
                      // Plugin_044_serial_buf[bytes_read] = 0;
                      break;
                  }
                }
                else
                {
                  Serial.read();      // when the buffer is full, just read remaining input, but do not store...
                  bytes_read = 0;
                  state = WAITING;    // reset
                }
                digitalWrite(STATUS_LED, 0);
                timeOut = RXWait; // if serial received, reset timeout counter
              }
              delay(1);
              timeOut--;
            }

            if (state == DONE) {
              if (checkDatagram(bytes_read)) {
                bytes_read++;
                Plugin_044_serial_buf[bytes_read] = '\r';
                bytes_read++;
                Plugin_044_serial_buf[bytes_read] = '\n';
                bytes_read++;
                Plugin_044_serial_buf[bytes_read] = 0;
                P1GatewayClient.write((const uint8_t*)Plugin_044_serial_buf, bytes_read);
                P1GatewayClient.flush();
                addLog(LOG_LEVEL_DEBUG, F("P1   : data send!"));
                blinkLED();

                if (Settings.UseRules)
                {
                  LoadTaskSettings(event->TaskIndex);
                  String eventString = ExtraTaskSettings.TaskDeviceName;
                  eventString += F("#Data");
                  rulesProcessing(eventString);
                }

              } else {
                addLog(LOG_LEVEL_DEBUG, F("P1   : Error: Invalid CRC, dropped data"));
              }

              bytes_read = 0;
              state = WAITING;
            }   // state == DONE
          }
          success = true;
        }
        break;
      }

  }
  return success;
}
void blinkLED() {
  digitalWrite(STATUS_LED, 1);
  delay(500);
  digitalWrite(STATUS_LED, 0);
}
/*
   validP1char
       checks whether the incoming character is a valid one for a P1 datagram. Returns false if not, which signals corrupt datagram
*/
bool validP1char(char ch) {
  if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '.') || (ch == '!') || (ch == 92) || (ch == 13) || (ch == '\n') || (ch == '(') || (ch == ')') || (ch == '-') || (ch == '*') || (ch == ':') )
  {
    return true;
  } else {
    addLog(LOG_LEVEL_DEBUG, F("P1   : Error: invalid char read from P1"));
    if (serialdebug) {
      Serial.print(F("faulty char>"));
      Serial.print(ch);
      Serial.println(F("<"));
    }
    return false;
  }
}

int FindCharInArrayRev(char array[], char c, int len) {
  for (int i = len - 1; i >= 0; i--) {
    if (array[i] == c) {
      return i;
    }
  }
  return -1;
}

/*
   CRC16
      based on code written by Jan ten Hove
     https://github.com/jantenhove/P1-Meter-ESP8266
*/
unsigned int CRC16(unsigned int crc, unsigned char *buf, int len)
{
  for (int pos = 0; pos < len; pos++)
  {
    crc ^= (unsigned int)buf[pos];    // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }

  return crc;
}

/*  checkDatagram
      checks whether the checksum of the data received from P1 matches the checksum attached to the
      telegram
     based on code written by Jan ten Hove
     https://github.com/jantenhove/P1-Meter-ESP8266
*/
bool checkDatagram(int len) {
  int startChar = FindCharInArrayRev(Plugin_044_serial_buf, '/', len);
  int endChar = FindCharInArrayRev(Plugin_044_serial_buf, '!', len);
  bool validCRCFound = false;

  if (!CRCcheck) return true;

  if (serialdebug) {
    Serial.print(F("input length: "));
    Serial.println(len);
    Serial.print("Start char \\ : ");
    Serial.println(startChar);
    Serial.print(F("End char ! : "));
    Serial.println(endChar);
  }

  if (endChar >= 0)
  {
    currCRC = CRC16(0x0000, (unsigned char *) Plugin_044_serial_buf, endChar - startChar + 1);

    char messageCRC[5];
    strncpy(messageCRC, Plugin_044_serial_buf + endChar + 1, 4);
    messageCRC[4] = 0;
    if (serialdebug) {
      for (int cnt = 0; cnt < len; cnt++)
        Serial.print(Plugin_044_serial_buf[cnt]);
    }

    validCRCFound = (strtoul(messageCRC, NULL, 16) == currCRC);
    if (!validCRCFound) {
      addLog(LOG_LEVEL_DEBUG, F("P1   : Error: invalid CRC found"));
    }
    currCRC = 0;
  }
  return validCRCFound;
}
