//#######################################################################################################
//#################################### Plugin-017: PN532 RFID reader ####################################
//#######################################################################################################

#define PLUGIN_017
#define PLUGIN_ID_017         17
#define PLUGIN_NAME_017       "RFID Reader - PN532"
#define PLUGIN_VALUENAME1_017 "Tag"

#define PN532_I2C_ADDRESS                   0x24

#define PN532_PREAMBLE                (0x00)
#define PN532_STARTCODE1              (0x00)
#define PN532_STARTCODE2              (0xFF)
#define PN532_POSTAMBLE               (0x00)
#define PN532_HOSTTOPN532             (0xD4)
#define PN532_PN532TOHOST             (0xD5)
#define PN532_ACK_WAIT_TIME           (10)  // ms, timeout of waiting for ACK
#define PN532_INVALID_ACK             (-1)
#define PN532_TIMEOUT                 (-2)
#define PN532_INVALID_FRAME           (-3)
#define PN532_NO_SPACE                (-4)

#define PN532_COMMAND_GETFIRMWAREVERSION    (0x02)
#define PN532_COMMAND_SAMCONFIGURATION      (0x14)
#define PN532_COMMAND_INLISTPASSIVETARGET   (0x4A)
#define PN532_RESPONSE_INLISTPASSIVETARGET  (0x4B)
#define PN532_MIFARE_ISO14443A              (0x00)

uint8_t Plugin_017_pn532_packetbuffer[64];
uint8_t Plugin_017_command;
boolean Plugin_017_init = false;

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

    case PLUGIN_WEBFORM_SHOW_VALUES:
      {
        string += F("<div class=\"div_l\">");
        string += ExtraTaskSettings.TaskDeviceValueNames[0];
        string += F(":</div><div class=\"div_r\">");
        string += (unsigned long)UserVar[event->BaseVarIndex] + ((unsigned long)UserVar[event->BaseVarIndex + 1] << 16);
        string += F("</div>");
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (!Plugin_017_init)
        {
          Plugin_017_init = true;
          Wire.beginTransmission(PN532_I2C_ADDRESS);
          delay(20);
          Wire.endTransmission();

          uint32_t versiondata = getFirmwareVersion();
          if (versiondata) {
            String log = F("PN532: Init: Found chip PN5");
            log += String((versiondata >> 24) & 0xFF, HEX);
            log += F(" FW: ");
            log += String((versiondata >> 16) & 0xFF, HEX);
            log += F(".");
            log += String((versiondata >> 8) & 0xFF, HEX);
            addLog(LOG_LEVEL_INFO, log);
          }

          Plugin_017_pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
          Plugin_017_pn532_packetbuffer[1] = 0x01; // normal mode;
          Plugin_017_pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
          Plugin_017_pn532_packetbuffer[3] = 0x01; // use IRQ pin!

          if (writeCommand(Plugin_017_pn532_packetbuffer, 4))
            return false;
        }

        uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
        uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
        success = readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

        if (success) {
          unsigned long key=uid[0];
          for (uint8_t i = 1; i < 4; i++) {
            key <<= 8;
            key += uid[i];
          }
          UserVar[event->BaseVarIndex] = (key & 0xFFFF);
          UserVar[event->BaseVarIndex + 1] = ((key >> 16) & 0xFFFF);
          String log = F("PN532: Tag: ");
          log += key;
          addLog(LOG_LEVEL_INFO, log);
          event->sensorType = SENSOR_TYPE_LONG;
          sendData(event);
        }
        break;
      }
  }
  return success;
}


/*********************************************************************************************\
 * PN532 get firmware version
\*********************************************************************************************/
uint32_t getFirmwareVersion(void)
{
  uint32_t response;

  Plugin_017_pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

  if (writeCommand(Plugin_017_pn532_packetbuffer, 1)) {
    return 0;
  }

  // read data packet
  int16_t status = readResponse(Plugin_017_pn532_packetbuffer, sizeof(Plugin_017_pn532_packetbuffer),10);
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


/*********************************************************************************************\
 * PN532 read tag
\*********************************************************************************************/
bool readPassiveTargetID(uint8_t cardbaudrate, uint8_t *uid, uint8_t *uidLength)
{
  byte timeout = 10;
  Plugin_017_pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
  Plugin_017_pn532_packetbuffer[1] = 1;  // max 1 cards at once (we can set this to 2 later)
  Plugin_017_pn532_packetbuffer[2] = cardbaudrate;

  if (writeCommand(Plugin_017_pn532_packetbuffer, 3)) {
    return 0x0;  // command failed
  }

  // read data packet
  if (readResponse(Plugin_017_pn532_packetbuffer, sizeof(Plugin_017_pn532_packetbuffer), timeout) < 0) {
    return 0x0;
  }

  if (Plugin_017_pn532_packetbuffer[0] != 1)
    return 0;

  uint16_t sens_res = Plugin_017_pn532_packetbuffer[2];
  sens_res <<= 8;
  sens_res |= Plugin_017_pn532_packetbuffer[3];

  /* Card appears to be Mifare Classic */
  *uidLength = Plugin_017_pn532_packetbuffer[5];

  for (uint8_t i = 0; i < Plugin_017_pn532_packetbuffer[5]; i++) {
    uid[i] = Plugin_017_pn532_packetbuffer[6 + i];
  }

  return 1;
}


/*********************************************************************************************\
 * PN532 write command
\*********************************************************************************************/
int8_t writeCommand(const uint8_t *header, uint8_t hlen)
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
  Wire.endTransmission();

  return readAckFrame();
}


/*********************************************************************************************\
 * PN532 read response
\*********************************************************************************************/
int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
  uint16_t time = 0;

  do {
    if (Wire.requestFrom(PN532_I2C_ADDRESS, len + 2)) {
      if (Wire.read() & 1) {  // check first byte --- status
        break;         // PN532 is ready
      }
    }

    delay(1);
    time++;
    if ((0 != timeout) && (time > timeout)) {
      return -1;
    }
  } while (1);

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
int8_t readAckFrame()
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

    delay(1);
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

