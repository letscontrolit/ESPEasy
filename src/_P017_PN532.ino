#ifdef USES_P017
//#######################################################################################################
//#################################### Plugin-017: PN532 RFID reader ####################################
//#######################################################################################################

#define PLUGIN_017
#define PLUGIN_ID_017         17
#define PLUGIN_NAME_017       "RFID - PN532"
#define PLUGIN_VALUENAME1_017 "Tag"

#define PN532_I2C_ADDRESS             0x24

#define PN532_PREAMBLE                (0x00)
#define PN532_STARTCODE1              (0x00)
#define PN532_STARTCODE2              (0xFF)
#define PN532_POSTAMBLE               (0x00)
#define PN532_HOSTTOPN532             (0xD4)
#define PN532_PN532TOHOST             (0xD5)
#define PN532_ACK_WAIT_TIME           (3)
#define PN532_INVALID_ACK             (-1)
#define PN532_TIMEOUT                 (-2)
#define PN532_INVALID_FRAME           (-3)
#define PN532_NO_SPACE                (-4)

#define PN532_COMMAND_GETFIRMWAREVERSION    (0x02)
#define PN532_COMMAND_SAMCONFIGURATION      (0x14)
#define PN532_COMMAND_INLISTPASSIVETARGET   (0x4A)
#define PN532_RESPONSE_INLISTPASSIVETARGET  (0x4B)
#define PN532_MIFARE_ISO14443A              (0x00)
#define PN532_COMMAND_POWERDOWN             (0x16)

uint8_t Plugin_017_pn532_packetbuffer[64];
uint8_t Plugin_017_command;

boolean Plugin_017(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_017;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_LONG;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_017);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_017));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
      	addFormPinSelect(F("Reset Pin"), F("taskdevicepin3"), Settings.TaskDevicePin3[event->TaskIndex]);
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        //set clock stretch to 2000, if its not set via advanced settings yet
        //something that Martinus figured out and added: https://github.com/esp8266/Arduino/issues/1541
        // if (!Settings.WireClockStretchLimit)
        //   Wire.setClockStretchLimit(2000);

        for(byte x=0; x < 3; x++)
        {
          if(Plugin_017_Init(Settings.TaskDevicePin3[event->TaskIndex]))
            break;
          delay(1000);
        }
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        static unsigned long tempcounter = 0;
        static byte counter;
        static byte errorCount=0;

        counter++;
        if (counter == 3 )
        {
          if (digitalRead(4) == 0 || digitalRead(5) == 0)
          {
            addLog(LOG_LEVEL_ERROR, F("PN532: BUS error"));
            Plugin_017_Init(Settings.TaskDevicePin3[event->TaskIndex]);
            // delay(1000);
          }
          counter = 0;
          uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
          uint8_t uidLength;
          byte error = Plugin_017_readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

          if (error == 1)
          {
            errorCount++;
            String log = F("PN532: Read error: ");
            log += errorCount;
            addLog(LOG_LEVEL_ERROR, log);
          }
          else
            errorCount=0;

          if (errorCount > 2) // if three consecutive I2C errors, reset PN532
          {
            Plugin_017_Init(Settings.TaskDevicePin3[event->TaskIndex]);
          }


          if (error == 0) {
            unsigned long key = uid[0];
            for (uint8_t i = 1; i < 4; i++) {
              key <<= 8;
              key += uid[i];
            }
            UserVar[event->BaseVarIndex] = (key & 0xFFFF);
            UserVar[event->BaseVarIndex + 1] = ((key >> 16) & 0xFFFF);
            String log = F("PN532: Tag: ");
            log += key;
            tempcounter++;
            log += " ";
            log += tempcounter;
            addLog(LOG_LEVEL_INFO, log);
            sendData(event);
          }
        }
        break;
      }
  }
  return success;
}


/*********************************************************************************************\
 * PN532 init
\*********************************************************************************************/
boolean Plugin_017_Init(int8_t resetPin)
{
  if (resetPin != -1)
  {
    String log = F("PN532: Reset on pin: ");
    log += resetPin;
    addLog(LOG_LEVEL_INFO, log);
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, LOW);
    delay(100);
    digitalWrite(resetPin, HIGH);
    pinMode(resetPin, INPUT_PULLUP);
    delay(10);
  }

  Wire.beginTransmission(PN532_I2C_ADDRESS);
  Wire.endTransmission();
  delay(5);

  uint32_t versiondata = getFirmwareVersion();
  if (versiondata) {
    String log = F("PN532: Found chip PN5");
    log += String((versiondata >> 24) & 0xFF, HEX);
    log += F(" FW: ");
    log += String((versiondata >> 16) & 0xFF, HEX);
    log += F(".");
    log += String((versiondata >> 8) & 0xFF, HEX);
    addLog(LOG_LEVEL_INFO, log);
  }
  else
    return false;

  Plugin_017_pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
  Plugin_017_pn532_packetbuffer[1] = 0x01; // normal mode;
  Plugin_017_pn532_packetbuffer[2] = 0x2; // timeout 50ms * 2 = 100 mS
  Plugin_017_pn532_packetbuffer[3] = 0x01; // use IRQ pin!

  if (Plugin_017_writeCommand(Plugin_017_pn532_packetbuffer, 4))
    return false;

  // to prevent nack on next read
  Wire.beginTransmission(PN532_I2C_ADDRESS);
  Wire.endTransmission();
  delay(5);

  return true;
}


/*********************************************************************************************\
 * PN532 get firmware version
\*********************************************************************************************/
uint32_t getFirmwareVersion(void)
{
  uint32_t response;

  Plugin_017_pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

  if (Plugin_017_writeCommand(Plugin_017_pn532_packetbuffer, 1)) {
    return 0;
  }

  delay(50); // we can try to read faster, but it will only put extra load on the I2C bus...

  // read data packet
  int16_t status = Plugin_017_readResponse(Plugin_017_pn532_packetbuffer, sizeof(Plugin_017_pn532_packetbuffer));
  if (0 > status) {
    return 0;
  }

  response = Plugin_017_pn532_packetbuffer[0];
  response <<= 8;
  response |= Plugin_017_pn532_packetbuffer[1];
  response <<= 8;
  response |= Plugin_017_pn532_packetbuffer[2];
  response <<= 8;
  response |= Plugin_017_pn532_packetbuffer[3];

  return response;
}


void Plugin_017_powerDown(void)
{

  Plugin_017_pn532_packetbuffer[0] = PN532_COMMAND_POWERDOWN;
  Plugin_017_pn532_packetbuffer[1] = 1 << 7; //allowed wakeup source is i2c

  if (Plugin_017_writeCommand(Plugin_017_pn532_packetbuffer, 2)) {
    return;
  }

  // delay(50); // we can try to read faster, but it will only put extra load on the I2C bus...

  // read and ignore response
  Plugin_017_readResponse(Plugin_017_pn532_packetbuffer, sizeof(Plugin_017_pn532_packetbuffer));
}


/*********************************************************************************************\
 * PN532 read tag
\*********************************************************************************************/
byte Plugin_017_readPassiveTargetID(uint8_t cardbaudrate, uint8_t *uid, uint8_t *uidLength)
{
  Plugin_017_pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
  Plugin_017_pn532_packetbuffer[1] = 1;  // max 1 cards at once
  Plugin_017_pn532_packetbuffer[2] = cardbaudrate;

  if (Plugin_017_writeCommand(Plugin_017_pn532_packetbuffer, 3)) {
    return 0x1;  // command failed
  }

  delay(50); // we can try to read faster, but it will only put extra load on the I2C bus...

  // read data packet
  if (Plugin_017_readResponse(Plugin_017_pn532_packetbuffer, sizeof(Plugin_017_pn532_packetbuffer)) < 0) {

    // if no tag read, need to clear something ?
    // it seems that without this code, the next read fails, taking another read to work again...

    // to prevent nack on next read
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    Wire.endTransmission();
    return 0x2;
  }

  if (Plugin_017_pn532_packetbuffer[0] != 1)
    return 0x3;

  uint16_t sens_res = Plugin_017_pn532_packetbuffer[2];
  sens_res <<= 8;
  sens_res |= Plugin_017_pn532_packetbuffer[3];

  /* Card appears to be Mifare Classic */
  *uidLength = Plugin_017_pn532_packetbuffer[5];

  for (uint8_t i = 0; i < Plugin_017_pn532_packetbuffer[5]; i++) {
    uid[i] = Plugin_017_pn532_packetbuffer[6 + i];
  }

  // Plugin_017_Init(-1);
  Plugin_017_powerDown();


  return 0;
}


/*********************************************************************************************\
 * PN532 write command
\*********************************************************************************************/
int8_t Plugin_017_writeCommand(const uint8_t *header, uint8_t hlen)
{
  Plugin_017_command = header[0];
  Wire.beginTransmission(PN532_I2C_ADDRESS);

  Wire.write(PN532_PREAMBLE);
  Wire.write(PN532_STARTCODE1);
  Wire.write(PN532_STARTCODE2);

  uint8_t length = hlen + 1;   // length of data field: TFI + DATA
  Wire.write(length);
  Wire.write(~length + 1);                 // checksum of length

  Wire.write(PN532_HOSTTOPN532);
  uint8_t sum = PN532_HOSTTOPN532;    // sum of TFI + DATA

  for (uint8_t i = 0; i < hlen; i++) {
    if (Wire.write(header[i])) {
      sum += header[i];

    } else {
      return PN532_INVALID_FRAME;
    }
  }

  uint8_t checksum = ~sum + 1;            // checksum of TFI + DATA
  Wire.write(checksum);
  Wire.write(PN532_POSTAMBLE);
  byte status = Wire.endTransmission();

  if (status != 0)
    return PN532_INVALID_FRAME;

  return Plugin_017_readAckFrame();
}


/*********************************************************************************************\
 * PN532 read response
\*********************************************************************************************/
int16_t Plugin_017_readResponse(uint8_t buf[], uint8_t len)
{
  if (!Wire.requestFrom(PN532_I2C_ADDRESS, len + 2))
    return -1;


  if (!(Wire.read() & 1))
    return -1;

  if (0x00 != Wire.read()      ||       // PREAMBLE
      0x00 != Wire.read()  ||       // STARTCODE1
      0xFF != Wire.read()           // STARTCODE2
     ) {

    return PN532_INVALID_FRAME;
  }

  uint8_t length = Wire.read();
  if (0 != (uint8_t)(length + Wire.read())) {   // checksum of length
    return PN532_INVALID_FRAME;
  }

  uint8_t cmd = Plugin_017_command + 1;               // response command
  if (PN532_PN532TOHOST != Wire.read() || (cmd) != Wire.read()) {
    return PN532_INVALID_FRAME;
  }

  length -= 2;
  if (length > len) {
    return PN532_NO_SPACE;  // not enough space
  }

  uint8_t sum = PN532_PN532TOHOST + cmd;
  for (uint8_t i = 0; i < length; i++) {
    buf[i] = Wire.read();
    sum += buf[i];

  }

  uint8_t checksum = Wire.read();
  if (0 != (uint8_t)(sum + checksum)) {
    return PN532_INVALID_FRAME;
  }
  Wire.read();         // POSTAMBLE

  return length;
}


/*********************************************************************************************\
 * PN532 read ack
\*********************************************************************************************/
int8_t Plugin_017_readAckFrame()
{
  const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
  uint8_t ackBuf[sizeof(PN532_ACK)];

  uint16_t time = 0;
  do {
    if (Wire.requestFrom(PN532_I2C_ADDRESS,  sizeof(PN532_ACK) + 1)) {
      if (Wire.read() & 1) {  // check first byte --- status
        break;         // PN532 is ready
      }
    }

    delay(5);
    time++;
    if (time > PN532_ACK_WAIT_TIME) {
      return PN532_TIMEOUT;
    }
  } while (1);


  for (uint8_t i = 0; i < sizeof(PN532_ACK); i++) {
    ackBuf[i] = Wire.read();
  }

  if (memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK))) {
    return PN532_INVALID_ACK;
  }

  return 0;
}
#endif // USES_P017
